---
type: "manual"
---

Generate comprehensive tests and examples for all Python and Lua bindings in the QtForge project. This should include:

1. **Test Coverage Requirements:**
   - Unit tests for every exposed Python binding function/class
   - Unit tests for every exposed Lua binding function/class
   - Integration tests that demonstrate cross-language interoperability
   - Edge case testing (null values, invalid parameters, boundary conditions)
   - Error handling tests for exception scenarios

2. **Example Requirements:**
   - Working code examples for each major binding feature
   - Step-by-step usage demonstrations
   - Real-world use case scenarios
   - Documentation examples that can be included in user guides

3. **Coverage Verification:**
   - Ensure 100% coverage of all public APIs exposed through bindings
   - Test both successful execution paths and error conditions
   - Verify memory management and resource cleanup in bindings
   - Test threading safety where applicable

4. **Organization:**
   - Place Python tests in appropriate test directories following project conventions
   - Place Lua tests in appropriate test directories following project conventions
   - Create separate example directories for Python and Lua with clear naming
   - Include README files explaining how to run tests and examples

5. **Quality Standards:**
   - All tests should be runnable and pass
   - Examples should be self-contained and executable
   - Include clear comments explaining the purpose of each test/example
   - Follow the project's existing coding standards and conventions
