# Platform-Specific Workflow Template

This document outlines the standard structure for platform-specific CI workflows in QtForge.

## Workflow Structure

### 1. Workflow Metadata

- **Name**: Clear, descriptive name indicating platform and purpose
- **Triggers**: Appropriate triggers for the platform
- **Environment Variables**: Platform-specific environment variables

### 2. Change Detection

- Use the shared `changes` job to determine if builds are needed
- Platform-specific path filters when appropriate

### 3. Build Matrix

- Platform-specific build configurations
- Compiler variations (MSVC, MinGW, GCC, Clang)
- Build types (Debug, Release)
- Architecture variations when applicable

### 4. Job Structure

Each platform workflow should include:

#### Build and Test Job

- **Setup Steps**:
  - Checkout code
  - Setup Qt (using shared action)
  - Setup build environment (using shared action)
  - Cache dependencies (using shared action)

- **Build Steps**:
  - Configure CMake with platform-specific settings
  - Build with performance monitoring
  - Run tests (if enabled)
  - Install artifacts

- **Artifact Steps**:
  - Upload build artifacts
  - Upload test results

#### Platform-Specific Jobs (Optional)

- Quality assurance (static analysis, security scans)
- Packaging (platform-specific package formats)
- Cross-compilation targets

### 5. Shared Components Usage

All workflows should use the shared components:

- `setup-qt.yml` - Qt installation and caching
- `setup-build-env.yml` - CMake, Ninja, compiler cache setup
- `cache-dependencies.yml` - Platform-specific dependency caching
- `build-and-test.yml` - Build and test execution
- `upload-artifacts.yml` - Artifact management

### 6. Platform-Specific Considerations

#### Windows

- MSVC and MinGW64 support
- Visual Studio generator
- Windows-specific packaging (NSIS, WIX, ZIP)
- Code signing support

#### Linux

- GCC and Clang support
- Ninja generator
- Linux-specific packaging (DEB, RPM, TGZ)
- AppImage and Flatpak support

#### macOS

- Xcode and Clang support
- macOS-specific packaging (DragNDrop, TGZ)
- Code signing and notarization support

### 7. Performance Optimization

- Intelligent caching strategies
- Parallel build configuration
- Build time monitoring
- Cache hit rate tracking

### 8. Error Handling

- Retry mechanisms for flaky operations
- Detailed error reporting
- Performance regression detection
- Workflow health monitoring

## Example Workflow Structure

```yaml
name: "Platform CI"

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main, develop]

env:
  QT_VERSION: "6.5.3"
  CMAKE_VERSION: "3.27.7"

jobs:
  changes:
    # Shared change detection logic

  build-and-test:
    needs: changes
    if: needs.changes.outputs.core == 'true'
    strategy:
      matrix:
        # Platform-specific matrix
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v4
      - uses: ./.github/workflows/shared/setup-qt
      - uses: ./.github/workflows/shared/setup-build-env
      - uses: ./.github/workflows/shared/cache-dependencies
      - uses: ./.github/workflows/shared/build-and-test
      - uses: ./.github/workflows/shared/upload-artifacts
```

## Best Practices

1. **Consistency**: Use shared components for common operations
2. **Performance**: Optimize caching and parallel execution
3. **Maintainability**: Keep platform-specific logic minimal
4. **Monitoring**: Include performance and health monitoring
5. **Documentation**: Document platform-specific requirements
6. **Testing**: Validate workflows with different scenarios
