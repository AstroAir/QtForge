# QtForge Tests Build Configuration

# Only build tests if explicitly requested
if not get_option('build_tests')
  subdir_done()
endif

message('Building QtForge tests')

# Test dependencies
test_deps = [qtforge_core_dep, qtforge_security_dep, qt6_core_dep]
if qt6_widgets_dep.found()
  test_deps += [qt6_widgets_dep]
endif
if qt6_network_dep.found()
  test_deps += [qt6_network_dep]
endif
if qt6_test_dep.found()
  test_deps += [qt6_test_dep]
endif

# Test include directories
test_inc = [inc_dir, include_directories('.')]

# Test helper function to create test executables
test_cpp_args = ['-DQTFORGE_TEST_BUILD']

# Core tests (always built)
subdir('core')
subdir('utils')
# subdir('security')  # Temporarily disabled due to incomplete types

# Communication tests (temporarily disabled due to MOC issues)
# subdir('communication')

# Manager tests (temporarily disabled due to MOC issues)
# subdir('managers')

# Conditional tests based on available Qt components
if qt6_network_dep.found()
  # Network-dependent tests
  message('Including network-dependent tests')
endif

if qt6_widgets_dep.found()
  # Widget-dependent tests
  message('Including widget-dependent tests')
endif

# Advanced feature tests
if get_option('enable_orchestration')
  subdir('orchestration')
endif

if get_option('enable_transactions')
  subdir('transactions')
endif

if get_option('enable_composition')
  subdir('composition')
endif

if get_option('enable_marketplace')
  subdir('marketplace')
endif

if get_option('enable_hot_reload')
  subdir('monitoring')
endif

# Python binding tests (temporarily disabled due to pybind11 issues)
# if get_option('python_bindings') and python3_dep.found() and pybind11_dep.found()
#   subdir('python')
# endif

# Integration tests (temporarily disabled due to dependencies)
# subdir('integration')

# Platform-specific tests (temporarily disabled due to dependencies)
# subdir('platform')

# Build system tests (directory doesn't exist yet)
# subdir('build_system')

# Bridge tests (if available)
if fs.exists('bridges/meson.build')
  subdir('bridges')
endif

# Install test data and utilities if requested
if get_option('install_tests')
  # Install test metadata files
  install_data([
    'mock_metadata.json',
    'test_metadata.json'
  ], install_dir: get_option('datadir') / 'qtforge' / 'tests')

  # Install test validation scripts
  install_data([
    'validate_structure.py',
    'verify_complete_coverage.py'
  ], install_dir: get_option('datadir') / 'qtforge' / 'tests')

  # Install test README
  install_data('README.md',
    install_dir: get_option('datadir') / 'qtforge' / 'tests'
  )
endif
