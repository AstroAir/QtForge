# Static Analysis Tools for QtForge - Comprehensive Comparison

**Document Version:** 1.0
**Date:** 2025-10-05
**Purpose:** Evaluate and compare static analysis tools for dead code detection and code quality analysis

---

## Executive Summary

This document provides a comprehensive comparison of static analysis tools suitable for the QtForge project, with a focus on dead code detection, build system integration, and Qt/C++20 compatibility.

### Top Recommendations

1. **🥇 clang-tidy** - Already integrated, excellent for dead code detection
2. **🥈 cppcheck** - Lightweight, easy to integrate, good for unused code
3. **🥉 include-what-you-use (IWYU)** - Excellent for include analysis

---

## Comparison Matrix

| Tool               | Type             | Qt Support   | Build Integration        | Dead Code       | False Positive Rate | License    | Recommendation                    |
| ------------------ | ---------------- | ------------ | ------------------------ | --------------- | ------------------- | ---------- | --------------------------------- |
| **clang-tidy**     | Compiler-based   | ✅ Excellent | ✅ compile_commands.json | ✅ Excellent    | 🟢 Low              | Apache 2.0 | ⭐⭐⭐⭐⭐ **HIGHLY RECOMMENDED** |
| **cppcheck**       | Static analyzer  | ✅ Good      | ⚠️ Manual config         | ✅ Good         | 🟡 Medium           | GPL-3.0    | ⭐⭐⭐⭐ **RECOMMENDED**          |
| **IWYU**           | Include analyzer | ✅ Excellent | ✅ compile_commands.json | ✅ Include-only | 🟢 Low              | NCSA       | ⭐⭐⭐⭐ **RECOMMENDED**          |
| **clang-analyzer** | Compiler-based   | ✅ Excellent | ✅ compile_commands.json | ✅ Good         | 🟢 Low              | Apache 2.0 | ⭐⭐⭐⭐ **RECOMMENDED**          |
| **SonarQube**      | Platform         | ✅ Good      | ✅ Multiple              | ✅ Excellent    | 🟡 Medium           | LGPL-3.0   | ⭐⭐⭐ **OPTIONAL**               |
| **PVS-Studio**     | Commercial       | ✅ Excellent | ✅ Multiple              | ✅ Excellent    | 🟢 Low              | Commercial | ⭐⭐⭐ **OPTIONAL**               |
| **Coverity**       | Commercial       | ✅ Excellent | ✅ Multiple              | ✅ Excellent    | 🟢 Low              | Commercial | ⭐⭐⭐ **OPTIONAL**               |
| **lizard**         | Complexity       | ⚠️ Limited   | ❌ None                  | ⚠️ Limited      | 🟡 Medium           | MIT        | ⭐⭐ **NOT RECOMMENDED**          |
| **CodeQL**         | Security-focused | ✅ Good      | ✅ GitHub Actions        | ⚠️ Limited      | 🟢 Low              | MIT        | ⭐⭐⭐ **ALREADY INTEGRATED**     |

---

## Detailed Tool Analysis

### 1. clang-tidy ⭐⭐⭐⭐⭐

**Status:** ✅ Already integrated in QtForge

**Strengths:**

- ✅ Compiler-based (uses clang AST)
- ✅ Excellent Qt support
- ✅ Integrates with compile_commands.json
- ✅ Low false positive rate
- ✅ Highly configurable
- ✅ Already configured in `.clang-tidy`
- ✅ Integrated in pre-commit hooks

**Dead Code Detection Capabilities:**

- Unused functions
- Unused variables
- Unused parameters
- Unreachable code
- Dead stores
- Unused includes (with misc-unused-using-decls)

**Configuration for Dead Code:**

```yaml
Checks: >
  -*,
  clang-analyzer-deadcode.*,
  misc-unused-*,
  readability-delete-null-pointer,
  readability-redundant-*,
  performance-unnecessary-*
```

**Integration:**

```bash
# Run clang-tidy on entire project
clang-tidy -p build src/**/*.cpp include/**/*.hpp

# Run with auto-fix
clang-tidy -p build --fix src/**/*.cpp

# Run specific checks
clang-tidy -p build --checks='-*,clang-analyzer-deadcode.*' src/**/*.cpp
```

**Recommendation:** ⭐⭐⭐⭐⭐ **PRIMARY TOOL** - Already integrated, just needs proper configuration

---

### 2. cppcheck ⭐⭐⭐⭐

**Status:** ❌ Not currently integrated

**Strengths:**

- ✅ Lightweight and fast
- ✅ Good Qt support
- ✅ Easy to integrate
- ✅ Standalone (no compiler needed)
- ✅ Good documentation
- ✅ Active development

**Dead Code Detection Capabilities:**

- Unused functions
- Unused variables
- Unreachable code
- Redundant code
- Unused struct members

**Configuration:**

```xml
<?xml version="1.0"?>
<project>
    <root name="QtForge"/>
    <builddir>build/cppcheck</builddir>
    <analyze-all-vs-configs>true</analyze-all-vs-configs>
    <check-headers>true</check-headers>
    <check-unused-templates>true</check-unused-templates>
    <max-ctu-depth>4</max-ctu-depth>
    <includedir>
        <dir name="include/"/>
        <dir name="src/"/>
    </includedir>
    <paths>
        <dir name="src/"/>
        <dir name="include/"/>
    </paths>
    <exclude>
        <path name="build/"/>
        <path name="third-party/"/>
    </exclude>
    <suppressions>
        <suppression>unusedFunction</suppression>
    </suppressions>
</project>
```

**Integration:**

```bash
# Install
sudo apt-get install cppcheck  # Linux
brew install cppcheck          # macOS
choco install cppcheck         # Windows

# Run analysis
cppcheck --project=cppcheck.xml --enable=all --inconclusive \
         --suppress=missingIncludeSystem --xml --output-file=cppcheck-report.xml

# Run with specific checks
cppcheck --enable=unusedFunction,style src/ include/
```

**Recommendation:** ⭐⭐⭐⭐ **SECONDARY TOOL** - Excellent complement to clang-tidy

---

### 3. include-what-you-use (IWYU) ⭐⭐⭐⭐

**Status:** ❌ Not currently integrated

**Strengths:**

- ✅ Excellent for include analysis
- ✅ Compiler-based (uses clang)
- ✅ Integrates with compile_commands.json
- ✅ Low false positive rate
- ✅ Excellent Qt support with mappings

**Dead Code Detection Capabilities:**

- Unused includes
- Forward declaration opportunities
- Include optimization
- Transitive include detection

**Configuration:**

```python
# iwyu_mappings.imp
[
  { include: ["<QObject>", "private", "<QObject>", "public"] },
  { include: ["<QString>", "private", "<QString>", "public"] },
  { include: ["<QList>", "private", "<QList>", "public"] },
  { symbol: ["QObject", "private", "<QObject>", "public"] },
  { symbol: ["QString", "private", "<QString>", "public"] },
]
```

**Integration:**

```bash
# Install
sudo apt-get install iwyu  # Linux
brew install include-what-you-use  # macOS

# Run analysis
iwyu_tool.py -p build -- -Xiwyu --mapping_file=iwyu_mappings.imp

# Apply fixes
fix_includes.py < iwyu_output.txt
```

**Recommendation:** ⭐⭐⭐⭐ **SPECIALIZED TOOL** - Essential for include hygiene

---

### 4. clang-analyzer ⭐⭐⭐⭐

**Status:** ⚠️ Partially integrated (via clang-tidy)

**Strengths:**

- ✅ Part of LLVM/clang
- ✅ Deep semantic analysis
- ✅ Excellent Qt support
- ✅ Low false positive rate
- ✅ Integrates with build system

**Dead Code Detection Capabilities:**

- Dead stores
- Dead assignments
- Unreachable code
- Unused values
- Logic errors

**Integration:**

```bash
# Run via scan-build
scan-build cmake -S . -B build
scan-build -o analysis-results cmake --build build

# Run via CodeChecker
CodeChecker analyze build/compile_commands.json -o reports
CodeChecker parse reports
```

**Recommendation:** ⭐⭐⭐⭐ **COMPLEMENTARY TOOL** - Use alongside clang-tidy

---

### 5. SonarQube/SonarCloud ⭐⭐⭐

**Status:** ❌ Not currently integrated

**Strengths:**

- ✅ Comprehensive platform
- ✅ Web-based dashboard
- ✅ Historical tracking
- ✅ Multiple language support
- ✅ CI/CD integration

**Weaknesses:**

- ⚠️ Requires server setup (SonarQube) or cloud account (SonarCloud)
- ⚠️ Medium false positive rate
- ⚠️ Can be slow on large projects
- ⚠️ Commercial features for advanced analysis

**Dead Code Detection Capabilities:**

- Unused code
- Duplicate code
- Code smells
- Technical debt tracking
- Coverage analysis

**Recommendation:** ⭐⭐⭐ **OPTIONAL** - Good for teams wanting comprehensive dashboards

---

### 6. PVS-Studio ⭐⭐⭐

**Status:** ❌ Not currently integrated

**Strengths:**

- ✅ Excellent C++ support
- ✅ Very low false positive rate
- ✅ Excellent Qt support
- ✅ Good documentation
- ✅ Active development

**Weaknesses:**

- ❌ Commercial license required
- ❌ Expensive for open-source projects
- ⚠️ Free tier has limitations

**Dead Code Detection Capabilities:**

- Unused code
- Unreachable code
- Dead stores
- Redundant operations
- Optimization opportunities

**Recommendation:** ⭐⭐⭐ **OPTIONAL** - Consider if budget allows

---

### 7. CodeQL ⭐⭐⭐

**Status:** ✅ Already integrated (GitHub Actions)

**Strengths:**

- ✅ Excellent security analysis
- ✅ Free for open-source
- ✅ GitHub integration
- ✅ Query-based analysis

**Weaknesses:**

- ⚠️ Primarily security-focused
- ⚠️ Limited dead code detection
- ⚠️ Requires GitHub

**Dead Code Detection Capabilities:**

- Unused variables (limited)
- Unreachable code (limited)
- Security-related dead code

**Recommendation:** ⭐⭐⭐ **KEEP FOR SECURITY** - Not primary dead code tool

---

## Recommended Tool Stack for QtForge

### Tier 1: Essential Tools (Already Integrated)

1. **clang-tidy** - Primary static analysis
   - Configure for dead code detection
   - Run in CI/CD pipeline
   - Integrate with pre-commit hooks

2. **CodeQL** - Security analysis
   - Keep current GitHub Actions integration
   - Focus on security issues

### Tier 2: Recommended Additions

3. **cppcheck** - Complementary analysis
   - Add to CI/CD pipeline
   - Run weekly on full codebase
   - Focus on unused code detection

4. **include-what-you-use** - Include hygiene
   - Run monthly or on-demand
   - Clean up include files
   - Reduce compilation times

### Tier 3: Optional Enhancements

5. **SonarCloud** - If team wants dashboards
6. **PVS-Studio** - If budget allows

---

## Integration Strategy

### Phase 1: Enhance Existing Tools (Week 1)

- ✅ Update clang-tidy configuration for dead code
- ✅ Add dead code checks to pre-commit hooks
- ✅ Document usage in CONTRIBUTING.md

### Phase 2: Add cppcheck (Week 2)

- ✅ Create cppcheck configuration
- ✅ Add to CI/CD pipeline
- ✅ Create baseline suppressions

### Phase 3: Add IWYU (Week 3)

- ✅ Create IWYU mappings for Qt
- ✅ Create cleanup scripts
- ✅ Run initial analysis and cleanup

### Phase 4: Monitoring and Refinement (Ongoing)

- ✅ Monitor false positive rates
- ✅ Tune configurations
- ✅ Update documentation

---

## Conclusion

**Recommended Approach:**

1. **Enhance clang-tidy** (already integrated) - Primary tool
2. **Add cppcheck** - Complementary analysis
3. **Add IWYU** - Include hygiene
4. **Keep CodeQL** - Security focus

This combination provides:

- ✅ Comprehensive coverage
- ✅ Low false positive rate
- ✅ Good Qt/C++20 support
- ✅ Build system integration
- ✅ Free and open-source
- ✅ Active maintenance

---

_This comparison is based on QtForge's specific needs: Qt6, C++20, multiple build systems, and emphasis on accuracy over coverage._
