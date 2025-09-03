---
type: "manual"
---

Update the Python and Lua bindings to match the latest C++ implementation, ensuring complete functional coverage. Specifically:

1. **Analyze the current C++ codebase** to identify all public APIs, classes, methods, and functions that should be exposed to Python and Lua
2. **Review existing Python bindings** to identify missing functionality, outdated method signatures, or deprecated APIs that need updating
3. **Review existing Lua bindings** to identify missing functionality, outdated method signatures, or deprecated APIs that need updating
4. **Update Python bindings** to include:
   - All new classes and methods added to the C++ implementation
   - Updated method signatures that have changed
   - New enums, constants, and data structures
   - Proper error handling and exception mapping
5. **Update Lua bindings** to include:
   - All new classes and methods added to the C++ implementation
   - Updated method signatures that have changed
   - New enums, constants, and data structures
   - Proper error handling
6. **Ensure consistency** between Python and Lua bindings where applicable
7. **Update or create tests** for both Python and Lua bindings to verify the new functionality works correctly
8. **Update documentation** or binding-specific README files to reflect the changes

Focus on maintaining backward compatibility where possible, and clearly document any breaking changes that are necessary.
