---
type: "manual"
---

Update the xmake build system to be compatible with the latest CMake modifications in the project. Please:

1. Analyze the current xmake.lua configuration file and identify any incompatibilities with recent CMake changes
2. Review the CMakeLists.txt files to understand what dependencies, targets, and build configurations have been modified
3. Update the xmake.lua file to match the current CMake build structure, including:
   - Source file lists and directory structures
   - Library dependencies and linking requirements
   - Compiler flags and build options
   - Target definitions and output configurations
4. Ensure that the xmake build system can successfully compile the entire project without errors
5. Test the build to verify that all components are properly linked and the resulting binaries function correctly

The goal is to maintain feature parity between the xmake and CMake build systems so that developers can use either build system interchangeably.
