---
type: "manual"
---

Run comprehensive tests for both Python and Lua bindings in the QtForge project to verify all functionality is working correctly. Follow these steps:

1. First, identify and locate all existing test files for Python and Lua bindings in the codebase
2. Run the Python binding tests using the appropriate test runner (ctest, or the project's preferred testing framework)
3. Run the Lua binding tests using the appropriate Lua testing framework or custom test scripts
4. Analyze any test failures or errors that occur during execution
5. For each failing test or error encountered:
   - Investigate the root cause by examining the binding code, test code, and any related dependencies
   - Implement fixes to resolve the issues
   - Re-run the specific tests to verify the fixes work
6. Continue this process until all tests pass successfully
7. Provide a summary of what was tested, what issues were found, and what fixes were implemented
8. Suggest running the tests again as a final verification step

Focus on ensuring that the bindings correctly expose the intended functionality and that there are no runtime errors, memory leaks, or API mismatches between the C++ core and the Python/Lua interfaces.
