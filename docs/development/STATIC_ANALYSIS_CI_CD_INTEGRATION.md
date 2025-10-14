# Static Analysis CI/CD Integration Guide

**Document Version:** 1.0
**Date:** 2025-10-05
**Purpose:** Guide for integrating static analysis tools into QtForge CI/CD pipelines

---

## Table of Contents

1. [Overview](#overview)
2. [GitHub Actions Integration](#github-actions-integration)
3. [Pre-commit Hooks](#pre-commit-hooks)
4. [Scheduled Analysis](#scheduled-analysis)
5. [Pull Request Checks](#pull-request-checks)
6. [Reporting and Notifications](#reporting-and-notifications)

---

## Overview

### Integration Strategy

QtForge uses a multi-layered approach to static analysis:

1. **🔍 Pre-commit Hooks** - Catch issues before commit
2. **🚀 Pull Request Checks** - Validate changes before merge
3. **📅 Scheduled Analysis** - Weekly full codebase scan
4. **📊 Continuous Monitoring** - Track metrics over time

### Current Status

✅ **Already Integrated:**

- clang-tidy (pre-commit hooks)
- CodeQL (GitHub Actions - security)
- ruff (Python linting)
- mypy (Python type checking)

📋 **Recommended Additions:**

- cppcheck (weekly scheduled)
- IWYU (monthly scheduled)
- Enhanced clang-tidy (PR checks)

---

## GitHub Actions Integration

### Workflow 1: Pull Request Static Analysis

Create `.github/workflows/static-analysis-pr.yml`:

```yaml
name: Static Analysis - Pull Request

on:
  pull_request:
    branches: [main, develop]
    paths:
      - "src/**"
      - "include/**"
      - "CMakeLists.txt"
      - ".clang-tidy"

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

env:
  QT_VERSION: "6.5.3"
  CMAKE_VERSION: "3.27.7"

jobs:
  clang-tidy:
    name: clang-tidy Analysis
    runs-on: ubuntu-latest

    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        with:
          fetch-depth: 0 # Full history for better analysis

      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: ${{ env.QT_VERSION }}
          cache: true

      - name: Install clang-tidy
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-tidy-14
          sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-14 100

      - name: Configure CMake
        run: |
          cmake -S . -B build \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DQTFORGE_BUILD_TESTS=OFF

      - name: Run clang-tidy on changed files
        run: |
          # Get list of changed C++ files
          git diff --name-only --diff-filter=ACMR origin/${{ github.base_ref }}...HEAD | \
            grep -E '\.(cpp|hpp)$' > changed_files.txt || true

          if [ -s changed_files.txt ]; then
            echo "Running clang-tidy on changed files:"
            cat changed_files.txt

            cat changed_files.txt | xargs clang-tidy -p build \
              --checks='-*,clang-analyzer-deadcode.*,misc-unused-*,readability-redundant-*,performance-unnecessary-*' \
              > clang-tidy-report.txt 2>&1 || true

            # Check for errors
            if grep -q "error:" clang-tidy-report.txt; then
              echo "::error::clang-tidy found errors"
              cat clang-tidy-report.txt
              exit 1
            fi

            # Check for warnings
            if grep -q "warning:" clang-tidy-report.txt; then
              echo "::warning::clang-tidy found warnings"
              cat clang-tidy-report.txt
            fi
          else
            echo "No C++ files changed"
          fi

      - name: Upload analysis results
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: clang-tidy-report
          path: clang-tidy-report.txt
          retention-days: 30

  cppcheck:
    name: cppcheck Analysis
    runs-on: ubuntu-latest

    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Install cppcheck
        run: |
          sudo apt-get update
          sudo apt-get install -y cppcheck

      - name: Run cppcheck
        run: |
          cppcheck --project=tools/static-analysis/cppcheck.xml \
            --enable=all \
            --inconclusive \
            --suppress=missingIncludeSystem \
            --xml \
            --output-file=cppcheck-report.xml 2>&1 || true

      - name: Check for errors
        run: |
          if grep -q 'severity="error"' cppcheck-report.xml; then
            echo "::error::cppcheck found errors"
            cat cppcheck-report.xml
            exit 1
          fi

      - name: Upload analysis results
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: cppcheck-report
          path: cppcheck-report.xml
          retention-days: 30
```

### Workflow 2: Scheduled Full Analysis

Create `.github/workflows/static-analysis-scheduled.yml`:

```yaml
name: Static Analysis - Scheduled

on:
  schedule:
    # Run every Sunday at 2 AM UTC
    - cron: "0 2 * * 0"
  workflow_dispatch: # Allow manual trigger

env:
  QT_VERSION: "6.5.3"

jobs:
  full-analysis:
    name: Full Codebase Analysis
    runs-on: ubuntu-latest
    timeout-minutes: 60

    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: ${{ env.QT_VERSION }}
          cache: true

      - name: Install analysis tools
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-tidy cppcheck iwyu

      - name: Configure CMake
        run: |
          cmake -S . -B build \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DQTFORGE_BUILD_TESTS=ON \
            -DQTFORGE_BUILD_LUA_BINDINGS=ON \
            -DQTFORGE_BUILD_PYTHON_BINDINGS=ON

      - name: Build project
        run: cmake --build build --parallel

      - name: Run all analysis tools
        run: |
          chmod +x tools/static-analysis/run_analysis.sh
          ./tools/static-analysis/run_analysis.sh --all

      - name: Generate summary
        run: |
          echo "# Static Analysis Summary" > analysis-summary.md
          echo "" >> analysis-summary.md
          echo "## clang-tidy" >> analysis-summary.md
          grep -c "warning:" analysis-reports/clang-tidy-report.txt >> analysis-summary.md || echo "0" >> analysis-summary.md
          echo "" >> analysis-summary.md
          echo "## cppcheck" >> analysis-summary.md
          grep -c 'severity="warning"' analysis-reports/cppcheck-report.xml >> analysis-summary.md || echo "0" >> analysis-summary.md

      - name: Upload reports
        uses: actions/upload-artifact@v3
        with:
          name: full-analysis-reports
          path: analysis-reports/
          retention-days: 90

      - name: Create issue if errors found
        if: failure()
        uses: actions/github-script@v7
        with:
          script: |
            github.rest.issues.create({
              owner: context.repo.owner,
              repo: context.repo.repo,
              title: 'Static Analysis Found Issues',
              body: 'The scheduled static analysis found issues. Please review the artifacts.',
              labels: ['static-analysis', 'automated']
            })
```

---

## Pre-commit Hooks

### Enhanced .pre-commit-config.yaml

Update the existing `.pre-commit-config.yaml` to include dead code checks:

```yaml
# Add to existing .pre-commit-config.yaml

# Enhanced clang-tidy with dead code detection
- repo: https://github.com/pocc/pre-commit-hooks
  rev: v1.3.5
  hooks:
    - id: clang-tidy
      files: \.(c|cc|cxx|cpp)$
      exclude: "^(third-party/.*|external/.*|vendor/.*|build/.*|.*_autogen/.*)$"
      args:
        - -p=build
        - --format-style=file
        - --checks=-*,clang-analyzer-deadcode.*,misc-unused-*,readability-redundant-*
      additional_dependencies: [clang-tidy]
```

### Local Pre-commit Setup

```bash
# Install pre-commit
pip install pre-commit

# Install hooks
pre-commit install

# Run on all files (first time)
pre-commit run --all-files

# Run on staged files (automatic on commit)
git commit -m "Your message"
```

---

## Scheduled Analysis

### Weekly Full Scan

**Purpose:** Catch issues that accumulate over time

**Configuration:**

- Runs every Sunday at 2 AM UTC
- Analyzes entire codebase
- All feature flags enabled
- Generates comprehensive reports

**Notifications:**

- Creates GitHub issue if errors found
- Uploads artifacts for review
- Sends summary to team (optional)

### Monthly Include Cleanup

**Purpose:** Maintain include hygiene

**Configuration:**

- Runs first Sunday of each month
- Uses IWYU to analyze includes
- Generates cleanup suggestions
- Creates PR with fixes (optional)

**Example Workflow:**

````yaml
name: Monthly Include Cleanup

on:
  schedule:
    - cron: "0 3 1 * *" # First day of month at 3 AM
  workflow_dispatch:

jobs:
  iwyu-cleanup:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install IWYU
        run: sudo apt-get install -y iwyu

      - name: Run IWYU
        run: |
          cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
          iwyu_tool.py -p build -- \
            -Xiwyu --mapping_file=tools/static-analysis/iwyu_mappings.imp \
            > iwyu-suggestions.txt

      - name: Create issue with suggestions
        uses: actions/github-script@v7
        with:
          script: |
            const fs = require('fs');
            const suggestions = fs.readFileSync('iwyu-suggestions.txt', 'utf8');
            github.rest.issues.create({
              owner: context.repo.owner,
              repo: context.repo.repo,
              title: 'Monthly Include Cleanup Suggestions',
              body: '```\n' + suggestions + '\n```',
              labels: ['maintenance', 'includes']
            });
````

---

## Pull Request Checks

### Required Checks

Configure branch protection to require:

1. ✅ **clang-tidy** - No errors allowed
2. ✅ **cppcheck** - No errors allowed
3. ⚠️ **Warnings** - Allowed but reported

### Optional Checks

1. **Code Coverage** - Track coverage changes
2. **Performance** - Benchmark critical paths
3. **Documentation** - Ensure docs updated

### Configuration

In GitHub repository settings:

```
Settings → Branches → Branch protection rules → main

☑ Require status checks to pass before merging
  ☑ clang-tidy Analysis
  ☑ cppcheck Analysis
  ☐ IWYU Analysis (optional)

☑ Require branches to be up to date before merging
```

---

## Reporting and Notifications

### Artifact Storage

**Retention Policy:**

- PR checks: 30 days
- Scheduled analysis: 90 days
- Release analysis: Permanent

**Organization:**

```
analysis-reports/
├── clang-tidy-report.txt
├── cppcheck-report.xml
├── cppcheck-html/
│   └── index.html
├── iwyu-report.txt
└── summary.md
```

### Notifications

#### Slack Integration (Optional)

```yaml
- name: Notify Slack
  if: failure()
  uses: 8398a7/action-slack@v3
  with:
    status: ${{ job.status }}
    text: "Static analysis found issues"
    webhook_url: ${{ secrets.SLACK_WEBHOOK }}
```

#### Email Notifications

```yaml
- name: Send email
  if: failure()
  uses: dawidd6/action-send-mail@v3
  with:
    server_address: smtp.gmail.com
    server_port: 465
    username: ${{ secrets.MAIL_USERNAME }}
    password: ${{ secrets.MAIL_PASSWORD }}
    subject: Static Analysis Failed
    body: Check the artifacts for details
    to: team@example.com
```

### Dashboard Integration

#### SonarCloud (Optional)

```yaml
- name: SonarCloud Scan
  uses: SonarSource/sonarcloud-github-action@master
  env:
    GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
    SONAR_TOKEN: ${{ secrets.SONAR_TOKEN }}
```

---

## Best Practices

### 1. Incremental Analysis

✅ **Do:**

- Analyze only changed files in PRs
- Run full analysis on schedule
- Cache build artifacts

❌ **Don't:**

- Analyze entire codebase on every PR
- Run expensive tools on every commit
- Ignore caching opportunities

### 2. Failure Handling

✅ **Do:**

- Fail on errors, warn on warnings
- Provide clear error messages
- Upload artifacts on failure

❌ **Don't:**

- Fail on warnings (too strict)
- Hide error details
- Delete artifacts on failure

### 3. Performance Optimization

✅ **Do:**

- Use parallel analysis
- Cache dependencies
- Limit analysis scope

❌ **Don't:**

- Run sequentially
- Rebuild dependencies
- Analyze everything always

---

## Troubleshooting

### Issue: Workflow times out

**Solution:**

```yaml
jobs:
  analyze:
    timeout-minutes: 60 # Increase timeout
    steps:
      - run: cmake --build build --parallel $(nproc) # Parallel build
```

### Issue: False positives in CI

**Solution:**

- Add suppressions to configuration files
- Use same tool versions locally and in CI
- Document known false positives

### Issue: Inconsistent results

**Solution:**

- Pin tool versions
- Use same build configuration
- Ensure clean build state

---

## Summary

### Recommended Setup

1. **Pre-commit:** clang-tidy on changed files
2. **PR Checks:** clang-tidy + cppcheck on changed files
3. **Weekly:** Full analysis with all tools
4. **Monthly:** IWYU include cleanup

### Metrics to Track

- Number of warnings over time
- False positive rate
- Time to fix issues
- Code coverage

### Success Criteria

- ✅ Zero errors in main branch
- ✅ Decreasing warning count
- ✅ Fast PR feedback (<5 minutes)
- ✅ Low false positive rate (<10%)

---

_This integration guide ensures consistent code quality while maintaining developer productivity._
