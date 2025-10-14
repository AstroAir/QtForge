# Static Analysis Best Practices for QtForge

**Document Version:** 1.0
**Date:** 2025-10-05
**Audience:** QtForge Contributors and Maintainers
**Purpose:** Establish best practices for running static analysis and dead code detection

---

## Table of Contents

1. [Overview](#overview)
2. [Pre-Analysis Checklist](#pre-analysis-checklist)
3. [Running Static Analysis](#running-static-analysis)
4. [Interpreting Results](#interpreting-results)
5. [Verification Process](#verification-process)
6. [Common Pitfalls](#common-pitfalls)
7. [Tool-Specific Guidelines](#tool-specific-guidelines)

---

## Overview

### Purpose of Static Analysis

Static analysis tools help identify:

- ✅ Dead code and unused functions
- ✅ Unused includes and headers
- ✅ Potential bugs and code smells
- ✅ Performance issues
- ✅ Security vulnerabilities

### Key Principles

1. **🎯 Accuracy Over Coverage** - Prefer tools with low false positive rates
2. **🔍 Always Verify** - Never trust tool output blindly
3. **🏗️ Build System Integration** - Use tools that understand your build system
4. **📊 Track Metrics** - Monitor false positive rates over time
5. **🔄 Iterate** - Continuously improve tool configurations

---

## Pre-Analysis Checklist

### Before Running Any Static Analysis Tool

- [ ] **Clean Build** - Ensure project builds successfully

  ```bash
  rm -rf build/
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build build --parallel
  ```

- [ ] **Generate compile_commands.json** - Required for compiler-based tools

  ```bash
  # CMake automatically generates this with CMAKE_EXPORT_COMPILE_COMMANDS=ON
  ls -la build/compile_commands.json
  ```

- [ ] **Update Dependencies** - Ensure all dependencies are current

  ```bash
  # Update Qt, build tools, and analysis tools
  sudo apt-get update && sudo apt-get upgrade  # Linux
  brew upgrade  # macOS
  ```

- [ ] **Check Tool Versions** - Use compatible tool versions

  ```bash
  clang-tidy --version  # Should be >= 14.0
  cppcheck --version    # Should be >= 2.10
  ```

- [ ] **Review Tool Configuration** - Verify configuration files are current
  ```bash
  cat .clang-tidy
  cat cppcheck.xml
  ```

### QtForge-Specific Considerations

- [ ] **Multiple Build Systems** - Choose which build system to analyze
  - CMake (recommended for analysis)
  - Meson (alternative)
  - XMake (alternative)

- [ ] **Qt Version** - Ensure Qt6 is properly detected

  ```bash
  qmake6 --version
  echo $CMAKE_PREFIX_PATH
  ```

- [ ] **Conditional Compilation** - Be aware of feature flags
  - `QTFORGE_BUILD_LUA_BINDINGS`
  - `QTFORGE_BUILD_PYTHON_BINDINGS`
  - `QTFORGE_BUILD_REMOTE_PLUGINS`

---

## Running Static Analysis

### Method 1: Using clang-tidy (Recommended)

**Full Project Analysis:**

```bash
# Run on all source files
clang-tidy -p build src/**/*.cpp include/**/*.hpp

# Run with specific checks
clang-tidy -p build \
  --checks='-*,clang-analyzer-deadcode.*,misc-unused-*,readability-redundant-*' \
  src/**/*.cpp

# Run with auto-fix (use with caution)
clang-tidy -p build --fix-errors src/**/*.cpp
```

**Incremental Analysis:**

```bash
# Analyze only changed files
git diff --name-only --diff-filter=ACMR | grep -E '\.(cpp|hpp)$' | \
  xargs clang-tidy -p build
```

**Parallel Analysis:**

```bash
# Use run-clang-tidy for parallel execution
run-clang-tidy -p build -j $(nproc) src/
```

### Method 2: Using cppcheck

**Full Project Analysis:**

```bash
# Run with all checks
cppcheck --project=cppcheck.xml \
  --enable=all \
  --inconclusive \
  --suppress=missingIncludeSystem \
  --xml \
  --output-file=cppcheck-report.xml

# Run with specific checks
cppcheck --enable=unusedFunction,style,performance \
  src/ include/
```

**Focused Analysis:**

```bash
# Check only for unused code
cppcheck --enable=unusedFunction \
  --suppress=unusedFunction:tests/* \
  src/ include/
```

### Method 3: Using include-what-you-use

**Full Project Analysis:**

```bash
# Run IWYU on all files
iwyu_tool.py -p build -- -Xiwyu --mapping_file=iwyu_mappings.imp > iwyu_output.txt

# Review suggestions
cat iwyu_output.txt

# Apply fixes (review first!)
fix_includes.py < iwyu_output.txt
```

**Single File Analysis:**

```bash
# Analyze specific file
include-what-you-use -p build src/core/plugin_manager.cpp
```

---

## Interpreting Results

### Understanding Output

#### clang-tidy Output Format

```
src/core/plugin_manager.cpp:123:5: warning: unused variable 'temp' [clang-analyzer-deadcode.DeadStores]
    int temp = 0;
    ^
```

**Components:**

- `src/core/plugin_manager.cpp:123:5` - File and location
- `warning` - Severity level
- `unused variable 'temp'` - Issue description
- `[clang-analyzer-deadcode.DeadStores]` - Check name

#### cppcheck Output Format

```xml
<error id="unusedFunction" severity="style" msg="The function 'helper' is never used.">
  <location file="src/utils/helper.cpp" line="45"/>
</error>
```

### Severity Levels

| Level           | Meaning        | Action Required        |
| --------------- | -------------- | ---------------------- |
| **error**       | Definite bug   | ✅ Fix immediately     |
| **warning**     | Likely issue   | ✅ Investigate and fix |
| **style**       | Code quality   | ⚠️ Consider fixing     |
| **performance** | Optimization   | ⚠️ Evaluate impact     |
| **portability** | Platform issue | ⚠️ Check if relevant   |
| **information** | FYI only       | ℹ️ Review              |

---

## Verification Process

### Step 1: Triage Results

**Categorize findings:**

1. **True Positives** - Real issues that need fixing
2. **False Positives** - Tool errors, need suppression
3. **Intentional** - Code is correct, needs documentation
4. **Unclear** - Requires investigation

### Step 2: Verify Each Finding

**For "Unused Function" warnings:**

```bash
# Search for function usage
git grep -n "function_name"

# Check if it's part of public API
grep -r "function_name" include/

# Check if it's used in tests
grep -r "function_name" tests/

# Check if it's exported for plugins
grep -r "Q_INVOKABLE.*function_name" include/
```

**For "Unused Include" warnings:**

```bash
# Try removing the include and rebuilding
# If build succeeds, include is truly unused

# Check if include is needed for forward declarations
# Check if include is needed for template instantiation
```

**For "Dead Code" warnings:**

```bash
# Check if code is conditionally compiled
grep -B5 -A5 "line_number" src/file.cpp | grep -E "#if|#ifdef|#ifndef"

# Check if code is platform-specific
# Check if code is feature-flag dependent
```

### Step 3: Document Decisions

**For suppressions:**

```cpp
// NOLINT(clang-analyzer-deadcode.DeadStores) - Intentional for debugging
int debug_counter = 0;

// cppcheck-suppress unusedFunction - Part of plugin API
void plugin_callback() {
    // ...
}
```

**For intentional code:**

```cpp
/**
 * @brief Helper function for future use
 * @note Currently unused but part of planned feature #123
 */
void future_feature() {
    // ...
}
```

---

## Common Pitfalls

### Pitfall 1: Analyzing Wrong Build Configuration

**Problem:** Tool analyzes debug build but production uses release build

**Solution:**

```bash
# Always analyze the same configuration you deploy
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
clang-tidy -p build src/**/*.cpp
```

### Pitfall 2: Ignoring Conditional Compilation

**Problem:** Tool reports code as unused when it's conditionally compiled

**Example:**

```cpp
#ifdef QTFORGE_LUA_BINDINGS
void lua_specific_function() {  // Reported as unused if Lua disabled
    // ...
}
#endif
```

**Solution:** Run analysis with all feature flags enabled

```bash
cmake -S . -B build \
  -DQTFORGE_BUILD_LUA_BINDINGS=ON \
  -DQTFORGE_BUILD_PYTHON_BINDINGS=ON \
  -DQTFORGE_BUILD_REMOTE_PLUGINS=ON
```

### Pitfall 3: Not Verifying File Existence

**Problem:** Tool reports files as missing when they exist

**Solution:** Always verify manually

```bash
# Check if file exists
ls -la src/path/to/file.cpp

# Check if file is in git
git ls-files | grep "file.cpp"
```

### Pitfall 4: Trusting Pattern-Based Tools

**Problem:** Tools using regex patterns have high false positive rates

**Solution:** Prefer compiler-based tools (clang-tidy, clang-analyzer)

### Pitfall 5: Not Understanding Qt Meta-Object System

**Problem:** Tool reports Qt slots/signals as unused

**Example:**

```cpp
class MyClass : public QObject {
    Q_OBJECT
private slots:
    void onTimeout();  // May be reported as unused
};
```

**Solution:** Suppress warnings for Qt meta-object methods

```cpp
// NOLINT - Qt slot connected via meta-object system
void onTimeout() {
    // ...
}
```

---

## Tool-Specific Guidelines

### clang-tidy Guidelines

**Configuration:**

```yaml
# .clang-tidy
Checks: >
  -*,
  clang-analyzer-deadcode.*,
  misc-unused-*,
  readability-redundant-*,
  performance-unnecessary-*

CheckOptions:
  - key: misc-unused-parameters.StrictMode
    value: true
```

**Best Practices:**

- ✅ Always use `-p build` to specify compile_commands.json
- ✅ Run on clean build
- ✅ Review auto-fixes before applying
- ✅ Use `--quiet` for less verbose output

### cppcheck Guidelines

**Configuration:**

```xml
<!-- cppcheck.xml -->
<project>
    <check-unused-templates>true</check-unused-templates>
    <max-ctu-depth>4</max-ctu-depth>
</project>
```

**Best Practices:**

- ✅ Use `--inconclusive` for more thorough analysis
- ✅ Suppress `missingIncludeSystem` for system headers
- ✅ Create baseline suppressions file
- ✅ Run with `--enable=all` initially, then tune

### IWYU Guidelines

**Best Practices:**

- ✅ Create Qt-specific mapping file
- ✅ Review suggestions before applying
- ✅ Run on stable codebase (not during active development)
- ✅ Test build after applying fixes

---

## Verification Checklist

Before accepting any static analysis finding as valid:

- [ ] **File Exists** - Verify file actually exists on filesystem
- [ ] **Reference Exists** - Verify reference actually exists in code
- [ ] **Build Succeeds** - Verify project builds before and after
- [ ] **Tests Pass** - Verify tests pass before and after
- [ ] **Manual Review** - Manually review the code in question
- [ ] **Context Check** - Check surrounding code for context
- [ ] **API Check** - Verify if part of public API
- [ ] **Plugin Check** - Verify if used by plugins
- [ ] **Conditional Check** - Check for conditional compilation
- [ ] **Qt Check** - Check for Qt meta-object usage

---

## Summary

### Golden Rules

1. **🎯 Always verify tool output manually**
2. **🏗️ Use compiler-based tools when possible**
3. **📊 Track false positive rates**
4. **🔄 Iterate on tool configurations**
5. **📝 Document all suppressions**
6. **✅ Test before and after changes**
7. **🤝 Review with team before major cleanups**

### Recommended Workflow

1. **Prepare** - Clean build, generate compile_commands.json
2. **Analyze** - Run tools with appropriate configurations
3. **Triage** - Categorize findings
4. **Verify** - Manually verify each finding
5. **Fix** - Apply fixes incrementally
6. **Test** - Run full test suite
7. **Review** - Code review before merging
8. **Document** - Update suppressions and documentation

---

_Following these best practices will help maintain high code quality while avoiding the pitfalls of false positives and incorrect analysis results._
