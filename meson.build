# QtForge Library Meson Build Configuration
# Modern C++ Plugin System for Qt Applications with Modular Architecture
# Version: 3.0.0

project('QtForge',
  'cpp',
  version: '3.0.0',
  license: 'Apache-2.0',
  meson_version: '>=0.63.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=false',
    'buildtype=release',
    'default_library=shared'
  ]
)

# Import required modules
qt6 = import('qt6')
fs = import('fs')
pkgconfig = import('pkgconfig')

# Project information
project_description = 'Modern C++ Plugin System for Qt Applications with Modular Architecture'
project_url = 'https://github.com/qtforge/qtforge'

# Version information
version_parts = meson.project_version().split('.')
version_major = version_parts[0].to_int()
version_minor = version_parts[1].to_int()
version_patch = version_parts[2].to_int()

# Global compile definitions
global_definitions = [
  '-DQTFORGE_VERSION_MAJOR=@0@'.format(version_major),
  '-DQTFORGE_VERSION_MINOR=@0@'.format(version_minor),
  '-DQTFORGE_VERSION_PATCH=@0@'.format(version_patch),
  '-DQTFORGE_VERSION_STRING="@0@"'.format(meson.project_version())
]

# Platform-specific definitions and flags
if host_machine.system() == 'windows'
  global_definitions += [
    '-DWIN32_LEAN_AND_MEAN',
    '-DNOMINMAX',
    '-D_CRT_SECURE_NO_WARNINGS'
  ]
  if get_option('default_library') == 'shared'
    global_definitions += ['-DQTFORGE_SHARED']
  else
    global_definitions += ['-DQTFORGE_STATIC']
  endif
elif host_machine.system() == 'linux'
  # Linux-specific flags will be added as needed
elif host_machine.system() == 'darwin'
  # macOS-specific flags will be added as needed
endif

# Compiler-specific flags
cpp_compiler = meson.get_compiler('cpp')
cpp_args = []
link_args = []

if cpp_compiler.get_id() == 'msvc'
  cpp_args += [
    '/utf-8',
    '/permissive-'
  ]
  if get_option('enable_warnings')
    cpp_args += ['/W4']
  endif
  if get_option('enable_werror')
    cpp_args += ['/WX']
  endif
  if get_option('enable_fast_math')
    cpp_args += ['/fp:fast']
  endif
else
  # GCC/Clang flags
  cpp_args += [
    '-fPIC'
  ]
  if get_option('enable_warnings')
    cpp_args += [
      '-Wall',
      '-Wextra',
      '-Wpedantic'
    ]
  endif
  if get_option('enable_werror')
    cpp_args += ['-Werror']
  endif
  if get_option('enable_fast_math')
    cpp_args += ['-ffast-math']
  endif

  # Sanitizers (debug builds only)
  if get_option('enable_sanitizers') and get_option('buildtype').startswith('debug')
    cpp_args += [
      '-fsanitize=address',
      '-fsanitize=undefined',
      '-fno-omit-frame-pointer'
    ]
    link_args += [
      '-fsanitize=address',
      '-fsanitize=undefined'
    ]
  endif
endif

# Link Time Optimization (release builds)
if get_option('enable_lto') and get_option('buildtype') == 'release'
  if cpp_compiler.get_id() == 'msvc'
    cpp_args += ['/GL']
    link_args += ['/LTCG']
  else
    cpp_args += ['-flto']
    link_args += ['-flto']
  endif
endif

# Build type specific definitions
buildtype_definitions = []
if get_option('buildtype').startswith('debug')
  buildtype_definitions += [
    '-DQTFORGE_DEBUG=1'
  ]
  if get_option('enable_component_logging')
    buildtype_definitions += ['-DQTFORGE_ENABLE_LOGGING=1']
  endif
else
  buildtype_definitions += [
    '-DQTFORGE_RELEASE=1',
    '-DNDEBUG=1'
  ]
endif

# Security hardening definitions
if get_option('enable_security_hardening')
  buildtype_definitions += ['-DQTFORGE_ENABLE_SECURITY_HARDENING=1']
endif

# Plugin profiling
if get_option('enable_plugin_profiling')
  buildtype_definitions += ['-DQTFORGE_ENABLE_PLUGIN_PROFILING=1']
endif

# Add global arguments
add_project_arguments(global_definitions, language: 'cpp')
add_project_arguments(buildtype_definitions, language: 'cpp')
add_project_arguments(cpp_args, language: 'cpp')
add_project_link_arguments(link_args, language: 'cpp')

# Qt6 dependency detection with method selection
qt_method = get_option('qt_method')

# Core Qt6 dependency (required)
qt6_core_dep = dependency('qt6', modules: ['Core'], required: true, method: qt_method)

# Optional Qt6 modules with force options
qt6_network_dep = dependency('qt6', modules: ['Network'],
  required: get_option('force_qt_network'), method: qt_method)
qt6_widgets_dep = dependency('qt6', modules: ['Widgets'],
  required: get_option('force_qt_widgets'), method: qt_method)
qt6_sql_dep = dependency('qt6', modules: ['Sql'],
  required: get_option('force_qt_sql'), method: qt_method)
qt6_concurrent_dep = dependency('qt6', modules: ['Concurrent'],
  required: get_option('force_qt_concurrent'), method: qt_method)
qt6_statemachine_dep = dependency('qt6', modules: ['StateMachine'],
  required: get_option('force_qt_statemachine'), method: qt_method)
qt6_test_dep = dependency('qt6', modules: ['Test'],
  required: false, method: qt_method)

# Feature detection based on available Qt modules
qt_features = {}
qt_features_definitions = []

if qt6_network_dep.found()
  qt_features += {'network': true}
  qt_features_definitions += ['-DQTFORGE_HAS_NETWORK']
  message('Qt6Network: Available')
else
  qt_features += {'network': false}
  message('Qt6Network: Not available')
endif

if qt6_widgets_dep.found()
  qt_features += {'widgets': true}
  qt_features_definitions += ['-DQTFORGE_HAS_WIDGETS']
  message('Qt6Widgets: Available')
else
  qt_features += {'widgets': false}
  message('Qt6Widgets: Not available')
endif

if qt6_sql_dep.found()
  qt_features += {'sql': true}
  qt_features_definitions += ['-DQTFORGE_HAS_SQL']
  message('Qt6Sql: Available')
else
  qt_features += {'sql': false}
  message('Qt6Sql: Not available')
endif

if qt6_concurrent_dep.found()
  qt_features += {'concurrent': true}
  qt_features_definitions += ['-DQTFORGE_HAS_CONCURRENT']
  message('Qt6Concurrent: Available')
else
  qt_features += {'concurrent': false}
  message('Qt6Concurrent: Not available')
endif

if qt6_statemachine_dep.found()
  qt_features += {'statemachine': true}
  qt_features_definitions += ['-DQTFORGE_HAS_STATEMACHINE']
  message('Qt6StateMachine: Available')
else
  qt_features += {'statemachine': false}
  message('Qt6StateMachine: Not available')
endif

# Add Qt feature definitions globally
add_project_arguments(qt_features_definitions, language: 'cpp')

# Check if Qt tools are available (with fallback for older Meson versions)
qt_tools_available = false
if meson.version().version_compare('>=1.6.0')
  qt_tools_available = qt6.has_tools(required: false, tools: ['moc', 'uic', 'rcc'])
else
  qt_tools_available = qt6.has_tools(required: false)
endif

if not qt_tools_available
  error('Required Qt6 tools (moc, uic, rcc) not found')
endif

# Include directories
inc_dir = include_directories('include')
src_inc_dir = include_directories('src')

# Python bindings with version checking
python3_dep = dependency('python3', version: '>=' + get_option('python_min_version'), required: get_option('python_bindings'))
pybind11_dep = dependency('pybind11', required: get_option('python_bindings'))

# Check Python version constraints
python_version_ok = true
if get_option('python_bindings') and python3_dep.found()
  python_version = python3_dep.version()
  if python_version.version_compare('>' + get_option('python_max_version'))
    warning('Python @0@ is above maximum supported version @1@'.format(python_version, get_option('python_max_version')))
  endif
endif

# Lua bindings with version checking
lua_dep = dependency('lua', version: '>=' + get_option('lua_min_version'), required: get_option('lua_bindings'))
sol2_dep = dependency('sol2', version: '>=' + get_option('sol2_min_version'), required: get_option('lua_bindings'))

# Manual sol2 detection if not found via pkg-config
if get_option('lua_bindings') and lua_dep.found() and not sol2_dep.found()
  sol2_include_dir = cpp_compiler.find_library('sol2', required: false)
  if not sol2_include_dir.found()
    # Try to find sol2 headers manually
    sol2_header = cpp_compiler.has_header('sol/sol.hpp',
      args: ['-I' + get_option('prefix') / 'include', '-I/usr/include', '-I/usr/local/include'])
    if sol2_header
      message('Found sol2 headers manually')
      sol2_dep = declare_dependency()
    endif
  endif
endif

# Build subdirectories
subdir('src')

# Optional components
if get_option('build_examples')
  subdir('examples')
endif

if get_option('build_tests')
  subdir('tests')
endif

# Check if bindings should be built (temporarily disabled)
if false  # get_option('python_bindings') and python3_dep.found() and pybind11_dep.found() and python_version_ok
  message('Building Python bindings')
  # Python bindings will be handled in src/python/meson.build
else
  message('Python bindings disabled or dependencies not found')
endif

if get_option('lua_bindings') and lua_dep.found() and sol2_dep.found()
  message('Building Lua bindings')
  # Lua bindings will be handled in src/lua/meson.build
else
  message('Lua bindings disabled or dependencies not found')
endif

# Benchmarking support
benchmark_dep = dependency('benchmark', required: get_option('build_benchmarks'))
if get_option('build_benchmarks') and benchmark_dep.found()
  message('Building benchmarks')
else
  if get_option('build_benchmarks')
    message('Benchmarks requested but Google Benchmark not found')
  endif
endif

# Testing framework detection
gtest_dep = dependency('gtest', required: false)
if get_option('build_tests') and not gtest_dep.found()
  # Fallback to Qt Test if GTest not available
  if qt6_test_dep.found()
    message('Using Qt6 Test framework')
  else
    message('No suitable test framework found')
  endif
elif gtest_dep.found()
  message('Found Google Test')
endif

# Documentation
doxygen = find_program('doxygen', required: get_option('build_docs'))
if get_option('build_docs') and doxygen.found()
  message('Documentation generation enabled')

  # Configure Doxygen
  doxygen_conf = configuration_data()
  doxygen_conf.set('PROJECT_NAME', meson.project_name())
  doxygen_conf.set('PROJECT_VERSION', meson.project_version())
  doxygen_conf.set('PROJECT_DESCRIPTION', project_description)
  doxygen_conf.set('INPUT_DIR', meson.current_source_dir() / 'include')
  doxygen_conf.set('OUTPUT_DIR', meson.current_build_dir() / 'docs')
  doxygen_conf.set('SOURCE_DIR', meson.current_source_dir())

  # Generate Doxyfile if template exists
  doxyfile_template = 'Doxyfile.in'
  if fs.exists(doxyfile_template)
    doxyfile = configure_file(
      input: doxyfile_template,
      output: 'Doxyfile',
      configuration: doxygen_conf
    )
  else
    # Use existing Doxyfile
    doxyfile = 'Doxyfile'
  endif

  # Create documentation target
  docs_target = custom_target('docs',
    input: doxyfile,
    output: 'html',
    command: [doxygen, '@INPUT@'],
    build_by_default: false,
    install: get_option('build_docs'),
    install_dir: get_option('datadir') / 'doc' / 'qtforge'
  )

  # Create alias
  alias_target('docs', docs_target)
endif

# Installation directories
install_includedir = get_option('includedir') / 'qtplugin'
install_libdir = get_option('libdir')
install_bindir = get_option('bindir')
install_docdir = get_option('datadir') / 'doc' / 'qtforge'

# Install headers
install_subdir('include/qtplugin',
  install_dir: get_option('includedir'),
  strip_directory: false
)

# Install documentation files
install_data([
  'README.md',
  'LICENSE',
  'CHANGELOG.md',
  'CODE_OF_CONDUCT.md',
  'CONTRIBUTING.md'
], install_dir: install_docdir)

# Install docs directory if it exists
if fs.is_dir('docs')
  install_subdir('docs',
    install_dir: install_docdir,
    strip_directory: false,
    exclude_files: ['*.pyc', '__pycache__']
  )
endif

# CMake compatibility configuration files (if requested)
if get_option('install_cmake_config')
  cmake_config_dir = get_option('libdir') / 'cmake' / 'QtForge'

  # Generate QtForgeConfig.cmake
  cmake_config_data = configuration_data()
  cmake_config_data.set('PROJECT_VERSION', meson.project_version())
  cmake_config_data.set('CMAKE_INSTALL_PREFIX', get_option('prefix'))
  cmake_config_data.set('CMAKE_INSTALL_LIBDIR', get_option('libdir'))
  cmake_config_data.set('CMAKE_INSTALL_INCLUDEDIR', get_option('includedir'))

  # Create CMake config content
  cmake_config_template = '''# QtForge CMake Configuration File
# Generated by Meson build system

set(QtForge_VERSION "@PROJECT_VERSION@")
set(QtForge_VERSION_MAJOR @VERSION_MAJOR@)
set(QtForge_VERSION_MINOR @VERSION_MINOR@)
set(QtForge_VERSION_PATCH @VERSION_PATCH@)

# Find Qt6 dependencies
find_package(Qt6 REQUIRED COMPONENTS Core)

# Import targets
include("${CMAKE_CURRENT_LIST_DIR}/QtForgeTargets.cmake")

# Create aliases for compatibility
if(NOT TARGET QtForge::Core)
    add_library(QtForge::Core ALIAS qtforge-core)
endif()

if(NOT TARGET QtForge::Security)
    add_library(QtForge::Security ALIAS qtforge-security)
endif()
'''

  cmake_config_data.set('VERSION_MAJOR', version_major)
  cmake_config_data.set('VERSION_MINOR', version_minor)
  cmake_config_data.set('VERSION_PATCH', version_patch)

  # Create a simple CMake config file using custom_target
  cmake_config_target = custom_target('QtForgeConfig.cmake',
    output: 'QtForgeConfig.cmake',
    command: [
      find_program('python3'), '-c',
      '''
import sys
content = """# QtForge CMake Configuration File
# Generated by Meson build system

set(QtForge_VERSION "@0@")
set(QtForge_VERSION_MAJOR @1@)
set(QtForge_VERSION_MINOR @2@)
set(QtForge_VERSION_PATCH @3@)

# Find Qt6 dependencies
find_package(Qt6 REQUIRED COMPONENTS Core)

# Create imported targets (simplified)
add_library(QtForge::Core SHARED IMPORTED)
add_library(QtForge::Security SHARED IMPORTED)
""".format("@0@", @1@, @2@, @3@)
with open(sys.argv[1], 'w') as f:
    f.write(content)
      '''.format(meson.project_version(), version_major, version_minor, version_patch),
      '@OUTPUT@'
    ],
    install: true,
    install_dir: cmake_config_dir
  )
endif

# Packaging support
if get_option('package_format') != 'none'
  # Create package metadata
  package_metadata = {
    'name': meson.project_name(),
    'version': meson.project_version(),
    'description': project_description,
    'vendor': get_option('package_vendor'),
    'maintainer': get_option('package_maintainer'),
    'license': meson.project_license()[0],
    'url': project_url
  }

  # Create packaging target based on format
  package_format = get_option('package_format')

  if package_format == 'zip'
    package_target = custom_target('package',
      output: 'QtForge-@0@.zip'.format(meson.project_version()),
      command: [
        find_program('python3'),
        '-c',
        '''
import zipfile
import os
import sys
with zipfile.ZipFile(sys.argv[1], 'w') as zf:
    for root, dirs, files in os.walk("@0@"):
        for file in files:
            zf.write(os.path.join(root, file))
        '''.format(meson.current_build_dir())
      ],
      build_by_default: false
    )
  elif package_format in ['tar.gz', 'tar.xz']
    compression = package_format.split('.')[1]
    package_target = custom_target('package',
      output: 'QtForge-@0@.@1@'.format(meson.project_version(), package_format),
      command: [
        find_program('tar'),
        '--create',
        '--@0@'.format(compression),
        '--file', '@OUTPUT@',
        '--directory', meson.current_build_dir(),
        '.'
      ],
      build_by_default: false
    )
  endif

  # Create alias
  alias_target('package', package_target)
endif

# Summary
summary({
  'Qt6 Core': qt6_core_dep.found(),
  'Qt6 Network': qt6_network_dep.found(),
  'Qt6 Widgets': qt6_widgets_dep.found(),
  'Qt6 Sql': qt6_sql_dep.found(),
  'Qt6 Concurrent': qt6_concurrent_dep.found(),
  'Qt6 StateMachine': qt6_statemachine_dep.found(),
  'Build Examples': get_option('build_examples'),
  'Build Tests': get_option('build_tests'),
  'Build Benchmarks': get_option('build_benchmarks'),
  'Build Component Tests': get_option('build_component_tests'),
  'Python Bindings': get_option('python_bindings') and python3_dep.found() and pybind11_dep.found(),
  'Lua Bindings': get_option('lua_bindings') and lua_dep.found() and sol2_dep.found(),
  'Documentation': get_option('build_docs') and doxygen.found(),
  'Library Type': get_option('default_library'),
  'Warnings': get_option('enable_warnings'),
  'Warnings as Errors': get_option('enable_werror'),
  'Sanitizers': get_option('enable_sanitizers'),
  'LTO': get_option('enable_lto'),
  'Fast Math': get_option('enable_fast_math'),
  'Security Hardening': get_option('enable_security_hardening'),
  'Component Logging': get_option('enable_component_logging'),
  'Plugin Profiling': get_option('enable_plugin_profiling'),
  'Hot Reload': get_option('enable_hot_reload'),
  'Metrics': get_option('enable_metrics'),
  'Transactions': get_option('enable_transactions'),
  'Orchestration': get_option('enable_orchestration'),
  'Composition': get_option('enable_composition'),
  'Marketplace': get_option('enable_marketplace'),
  'Plugin Sandbox': get_option('enable_plugin_sandbox'),
}, section: 'QtForge Configuration')
