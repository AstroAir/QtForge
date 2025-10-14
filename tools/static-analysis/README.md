# QtForge Static Analysis Tools

This directory contains configuration files and scripts for running static analysis on the QtForge codebase.

## Quick Start

### Linux/macOS

```bash
# Run all analysis tools
./tools/static-analysis/run_analysis.sh --all

# Run specific tool
./tools/static-analysis/run_analysis.sh --clang-tidy
./tools/static-analysis/run_analysis.sh --cppcheck
./tools/static-analysis/run_analysis.sh --iwyu
```

### Windows

```cmd
REM Run all analysis tools
tools\static-analysis\run_analysis.bat --all

REM Run specific tool
tools\static-analysis\run_analysis.bat --clang-tidy
tools\static-analysis\run_analysis.bat --cppcheck
```

## Prerequisites

### Required

- **CMake 3.21+** - Build system
- **Qt6** - Framework
- **Compiler** - GCC 10+, Clang 12+, or MSVC 2019+

### Analysis Tools

#### clang-tidy (Recommended)

**Linux:**

```bash
sudo apt-get install clang-tidy
```

**macOS:**

```bash
brew install llvm
```

**Windows:**

- Download LLVM from https://llvm.org/builds/
- Add to PATH

#### cppcheck (Recommended)

**Linux:**

```bash
sudo apt-get install cppcheck
```

**macOS:**

```bash
brew install cppcheck
```

**Windows:**

- Download from https://cppcheck.sourceforge.io/
- Add to PATH

#### include-what-you-use (Optional)

**Linux:**

```bash
sudo apt-get install iwyu
```

**macOS:**

```bash
brew install include-what-you-use
```

**Windows:**

- Not commonly available
- Use WSL or Linux VM

## Configuration Files

### cppcheck.xml

Configuration for cppcheck static analyzer.

**Features:**

- Dead code detection
- Unused function detection
- Style checking
- Performance analysis
- Qt-specific suppressions

**Customization:**

```xml
<!-- Add custom suppressions -->
<suppressions>
    <suppression>unusedFunction:path/to/file.cpp</suppression>
</suppressions>

<!-- Add custom defines -->
<defines>
    <define name="MY_FEATURE=1"/>
</defines>
```

### iwyu_mappings.imp

Include-what-you-use mapping file for Qt6.

**Features:**

- Qt6 header mappings
- Standard library mappings
- C++20 concepts/ranges mappings

**Customization:**

```python
# Add custom mappings
{ include: ["<MyHeader>", "private", "<MyHeader>", "public"] },
{ symbol: ["MyClass", "private", "<MyHeader>", "public"] },
```

### .clang-tidy (Project Root)

Configuration for clang-tidy analyzer (already exists in project root).

**Dead Code Checks:**

- `clang-analyzer-deadcode.*` - Dead code detection
- `misc-unused-*` - Unused code detection
- `readability-redundant-*` - Redundant code detection
- `performance-unnecessary-*` - Unnecessary operations

## Usage Examples

### Basic Analysis

```bash
# Run clang-tidy on entire project
./tools/static-analysis/run_analysis.sh --clang-tidy

# Run cppcheck on entire project
./tools/static-analysis/run_analysis.sh --cppcheck

# Run IWYU on entire project
./tools/static-analysis/run_analysis.sh --iwyu
```

### Advanced Usage

```bash
# Run with auto-fix (use with caution!)
./tools/static-analysis/run_analysis.sh --clang-tidy --fix

# Run with custom number of parallel jobs
./tools/static-analysis/run_analysis.sh --clang-tidy --jobs 8

# Run all tools
./tools/static-analysis/run_analysis.sh --all
```

### Manual Tool Invocation

#### clang-tidy

```bash
# Single file
clang-tidy -p build src/core/plugin_manager.cpp

# With specific checks
clang-tidy -p build \
  --checks='-*,clang-analyzer-deadcode.*,misc-unused-*' \
  src/core/plugin_manager.cpp

# With auto-fix
clang-tidy -p build --fix-errors src/core/plugin_manager.cpp
```

#### cppcheck

```bash
# Using configuration file
cppcheck --project=tools/static-analysis/cppcheck.xml \
  --enable=all \
  --inconclusive \
  --xml \
  --output-file=cppcheck-report.xml

# Specific checks
cppcheck --enable=unusedFunction,style src/ include/
```

#### include-what-you-use

```bash
# Using mapping file
iwyu_tool.py -p build -- \
  -Xiwyu --mapping_file=tools/static-analysis/iwyu_mappings.imp \
  > iwyu-report.txt

# Apply fixes
fix_includes.py < iwyu-report.txt
```

## Output Reports

All reports are saved to `analysis-reports/` directory:

- `clang-tidy-report.txt` - clang-tidy findings
- `cppcheck-report.xml` - cppcheck findings (XML format)
- `cppcheck-html/` - cppcheck HTML report (if available)
- `iwyu-report.txt` - IWYU findings

## Interpreting Results

### clang-tidy Output

```
src/core/plugin_manager.cpp:123:5: warning: unused variable 'temp' [clang-analyzer-deadcode.DeadStores]
    int temp = 0;
    ^
```

**Components:**

- File and line number
- Severity (warning/error)
- Issue description
- Check name in brackets

### cppcheck Output

```xml
<error id="unusedFunction" severity="style" msg="The function 'helper' is never used.">
  <location file="src/utils/helper.cpp" line="45"/>
</error>
```

**Components:**

- Error ID
- Severity level
- Message
- File location

### IWYU Output

```
src/core/plugin_manager.cpp should add these lines:
#include <memory>

src/core/plugin_manager.cpp should remove these lines:
- #include <vector>  // lines 10-10
```

**Components:**

- Files to modify
- Includes to add
- Includes to remove

## Best Practices

### Before Running Analysis

1. ✅ Clean build the project
2. ✅ Generate `compile_commands.json`
3. ✅ Update all dependencies
4. ✅ Review tool configurations

### During Analysis

1. ✅ Run tools on clean codebase
2. ✅ Use appropriate tool for the task
3. ✅ Review output carefully
4. ✅ Verify findings manually

### After Analysis

1. ✅ Triage results (true/false positives)
2. ✅ Verify each finding
3. ✅ Apply fixes incrementally
4. ✅ Test after each change
5. ✅ Document suppressions

## Common Issues

### Issue: compile_commands.json not found

**Solution:**

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Issue: Tool reports false positives

**Solution:**

- Verify findings manually
- Add suppressions to configuration
- Check for conditional compilation
- Review Qt meta-object usage

### Issue: Tool reports existing files as missing

**Solution:**

- Verify file exists: `ls -la path/to/file`
- Check git: `git ls-files | grep filename`
- Rebuild project
- Update tool version

## Integration with CI/CD

### GitHub Actions Example

```yaml
name: Static Analysis

on: [push, pull_request]

jobs:
  analyze:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install tools
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-tidy cppcheck

      - name: Build
        run: |
          cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
          cmake --build build

      - name: Run analysis
        run: |
          ./tools/static-analysis/run_analysis.sh --all

      - name: Upload reports
        uses: actions/upload-artifact@v3
        with:
          name: analysis-reports
          path: analysis-reports/
```

## Further Reading

- [Static Analysis Best Practices](../../docs/development/STATIC_ANALYSIS_BEST_PRACTICES.md)
- [Tool Comparison](../../docs/development/STATIC_ANALYSIS_TOOLS_COMPARISON.md)
- [Root Cause Analysis](../../docs/development/DEAD_CODE_ANALYSIS_ROOT_CAUSE.md)
- [clang-tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)
- [cppcheck Manual](https://cppcheck.sourceforge.io/manual.pdf)
- [IWYU Documentation](https://github.com/include-what-you-use/include-what-you-use)

## Support

For questions or issues:

1. Check the documentation in `docs/development/`
2. Review existing GitHub issues
3. Create a new issue with analysis reports attached

---

_Remember: Static analysis tools are helpers, not replacements for human judgment. Always verify findings manually!_
