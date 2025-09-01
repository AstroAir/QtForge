# QtForge Build System Migration Guide

This guide helps developers migrate from CMake or XMake to the new Meson build system for QtForge.

## Build System Comparison

### Command Comparison

| Task | CMake | XMake | Meson |
|------|-------|-------|-------|
| Configure | `cmake -B build` | `xmake config` | `meson setup builddir` |
| Build | `cmake --build build` | `xmake build` | `meson compile -C builddir` |
| Test | `ctest --test-dir build` | `xmake test` | `meson test -C builddir` |
| Install | `cmake --install build` | `xmake install` | `meson install -C builddir` |
| Clean | `cmake --build build --target clean` | `xmake clean` | `meson compile -C builddir --clean` |

### Configuration Options Mapping

#### CMake → Meson
```bash
# CMake
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DQTFORGE_BUILD_EXAMPLES=ON

# Meson equivalent
meson setup builddir -Dbuildtype=debug -Dbuild_examples=true
```

#### XMake → Meson
```bash
# XMake
xmake config --mode=debug --examples=y

# Meson equivalent
meson setup builddir -Dbuildtype=debug -Dbuild_examples=true
```

### Option Name Mapping

| Feature | CMake | XMake | Meson |
|---------|-------|-------|-------|
| Build Type | `CMAKE_BUILD_TYPE` | `--mode` | `buildtype` |
| Examples | `QTFORGE_BUILD_EXAMPLES` | `--examples` | `build_examples` |
| Tests | `QTFORGE_BUILD_TESTS` | `--tests` | `build_tests` |
| Python | `QTFORGE_BUILD_PYTHON_BINDINGS` | `--python_bindings` | `python_bindings` |
| Shared Libs | `BUILD_SHARED_LIBS` | `--shared` | `default_library` |
| Qt Method | `Qt6_DIR` | N/A | `qt_method` |

## Migration Steps

### Step 1: Install Meson
```bash
# Using pip
pip install meson ninja

# Using package manager (Ubuntu/Debian)
sudo apt install meson ninja-build

# Using package manager (Windows/MSYS2)
pacman -S mingw-w64-x86_64-meson mingw-w64-x86_64-ninja
```

### Step 2: Basic Migration
1. **Backup existing build directories**
   ```bash
   mv build build-cmake-backup
   mv .xmake .xmake-backup
   ```

2. **Try basic Meson build**
   ```bash
   meson setup builddir
   meson compile -C builddir
   ```

3. **Compare outputs**
   - Check that all libraries are built
   - Verify examples compile (if enabled)
   - Test basic functionality

### Step 3: Migrate Build Scripts

#### CI/CD Pipeline Migration

**GitHub Actions (CMake → Meson)**
```yaml
# Before (CMake)
- name: Configure CMake
  run: cmake -B build -DCMAKE_BUILD_TYPE=Release
- name: Build
  run: cmake --build build
- name: Test
  run: ctest --test-dir build

# After (Meson)
- name: Setup Meson
  run: meson setup builddir -Dbuildtype=release
- name: Build
  run: meson compile -C builddir
- name: Test
  run: meson test -C builddir
```

**Build Scripts Migration**
```bash
# Before (build.sh with CMake)
#!/bin/bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DQTFORGE_BUILD_EXAMPLES=ON
cmake --build build --parallel
cmake --install build --prefix /opt/qtforge

# After (build.sh with Meson)
#!/bin/bash
meson setup builddir -Dbuildtype=release -Dbuild_examples=true
meson compile -C builddir
meson install -C builddir --destdir /opt/qtforge
```

### Step 4: IDE Integration

#### Visual Studio Code
1. Install "Meson Build" extension
2. Update `.vscode/tasks.json`:
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Meson: Configure",
      "type": "shell",
      "command": "meson",
      "args": ["setup", "builddir"],
      "group": "build"
    },
    {
      "label": "Meson: Build",
      "type": "shell",
      "command": "meson",
      "args": ["compile", "-C", "builddir"],
      "group": {
        "kind": "build",
        "isDefault": true
      }
    }
  ]
}
```

#### Qt Creator
1. Use "Plain C++ Project" import
2. Configure custom build steps:
   - Build: `meson compile -C builddir`
   - Clean: `meson compile -C builddir --clean`
   - Run: Use built executables from `builddir/`

### Step 5: Packaging Migration

#### CMake CPack → Meson
```bash
# Before (CMake/CPack)
cmake --build build --target package

# After (Meson)
meson setup builddir -Dpackage_format=tar.xz
meson compile -C builddir package
```

## Common Migration Issues

### Issue 1: Qt6 Detection
**Problem**: Qt6 not found during Meson setup
**Solution**:
```bash
# Set PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/path/to/qt6/lib/pkgconfig:$PKG_CONFIG_PATH

# Or specify Qt method
meson setup builddir -Dqt_method=qmake
```

### Issue 2: MOC Processing
**Problem**: MOC files not generated correctly
**Solution**: Ensure headers with Q_OBJECT are listed in `qtforge_core_moc_headers` in `src/meson.build`

### Issue 3: Python Bindings
**Problem**: Python bindings fail to build
**Solution**:
```bash
# Install pybind11 first
pip install pybind11

# Then configure
meson setup builddir -Dpython_bindings=true
```

### Issue 4: Cross-compilation
**Problem**: Cross-compilation setup differs from CMake
**Solution**: Create Meson cross-files instead of CMake toolchain files
```ini
# cross/mingw-w64.txt
[binaries]
c = 'x86_64-w64-mingw32-gcc'
cpp = 'x86_64-w64-mingw32-g++'
ar = 'x86_64-w64-mingw32-ar'
strip = 'x86_64-w64-mingw32-strip'

[host_machine]
system = 'windows'
cpu_family = 'x86_64'
cpu = 'x86_64'
endian = 'little'
```

## Feature Parity Checklist

- [x] Core library building
- [x] Security library building  
- [x] Qt6 integration with MOC/UIC/RCC
- [x] Example building
- [x] Test building and execution
- [x] Python bindings
- [x] Documentation generation
- [x] Installation rules
- [x] Pkg-config generation
- [x] CMake compatibility files
- [x] Cross-compilation support
- [x] Packaging support

## Performance Comparison

Based on typical QtForge builds:

| Metric | CMake | XMake | Meson |
|--------|-------|-------|-------|
| Configure Time | ~15s | ~8s | ~5s |
| Clean Build | ~120s | ~90s | ~80s |
| Incremental Build | ~20s | ~15s | ~12s |
| Memory Usage | High | Medium | Low |

## Recommendations

### When to Use Each Build System

**Use Meson when:**
- Starting new development
- Want fast build times
- Need clean, readable build files
- Cross-compilation is important
- Qt6 integration is primary concern

**Keep CMake when:**
- Existing complex CMake setup works well
- Heavy IDE integration requirements
- Complex packaging needs
- Team expertise is primarily CMake

**Keep XMake when:**
- Lua scripting capabilities are needed
- Package management integration is important
- Cross-platform package distribution

### Migration Timeline Recommendation

1. **Week 1-2**: Install Meson, test basic builds
2. **Week 3-4**: Migrate development workflows
3. **Week 5-6**: Update CI/CD pipelines
4. **Week 7-8**: Team training and documentation
5. **Week 9+**: Full migration, deprecate old systems

## Support and Resources

- **Meson Documentation**: https://mesonbuild.com/
- **Qt6 Meson Module**: https://mesonbuild.com/Qt6-module.html
- **QtForge Meson Issues**: Use project issue tracker
- **Migration Support**: Contact QtForge maintainers

## Rollback Plan

If migration issues occur:
1. Restore backup build directories
2. Use original CMake/XMake commands
3. Report issues to QtForge maintainers
4. Consider gradual migration approach
