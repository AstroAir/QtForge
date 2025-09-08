# QtForge Python Bindings Build Configuration

# Python bindings are only built if explicitly requested and dependencies are available
if not get_option('python_bindings') or not python3_dep.found() or not pybind11_dep.found()
  subdir_done()
endif

message('Building QtForge Python bindings')

# Python installation
python3 = import('python').find_installation('python3')

# Python binding sources - progressive architecture
python_binding_sources = [
  'qtforge_python_progressive.cpp',
  'qt_conversions.cpp',
  'core/core_bindings_basic.cpp',
  'utils/utils_bindings.cpp',
  'security/security_bindings.cpp',
  'managers/managers_bindings.cpp',
  'orchestration/orchestration_bindings.cpp',
  'threading/threading_bindings.cpp',
  'monitoring/monitoring_bindings.cpp',
  'transactions/transaction_bindings.cpp',
  'composition/composition_bindings.cpp',
  'marketplace/marketplace_bindings.cpp'
]

# Communication bindings (conditional on Qt Network)
if qt6_network_dep.found()
  python_binding_sources += ['communication/communication_bindings.cpp']
endif

# Python binding dependencies
python_binding_deps = [
  qtforge_core_dep,
  qtforge_security_dep,
  qt6_core_dep,
  python3_dep,
  pybind11_dep
]

# Add optional Qt dependencies
if qt6_network_dep.found()
  python_binding_deps += [qt6_network_dep]
endif
if qt6_widgets_dep.found()
  python_binding_deps += [qt6_widgets_dep]
endif
if qt6_sql_dep.found()
  python_binding_deps += [qt6_sql_dep]
endif

# Python binding compile definitions
python_binding_definitions = [
  '-DQTPLUGIN_VERSION_MAJOR=@0@'.format(version_major),
  '-DQTPLUGIN_VERSION_MINOR=@0@'.format(version_minor),
  '-DQTPLUGIN_VERSION_PATCH=@0@'.format(version_patch),
  '-DQTFORGE_PYTHON_BINDINGS'
]

# Add feature-based definitions
if qt6_network_dep.found()
  python_binding_definitions += ['-DQTFORGE_NETWORK_AVAILABLE']
endif
if qt6_widgets_dep.found()
  python_binding_definitions += ['-DQTFORGE_WIDGETS_AVAILABLE']
endif
if qt6_sql_dep.found()
  python_binding_definitions += ['-DQTFORGE_SQL_AVAILABLE']
endif

# Create Python extension module
qtforge_python_module = python3.extension_module('qtforge',
  python_binding_sources,
  include_directories: [inc_dir, src_inc_dir, include_directories('.')],
  dependencies: python_binding_deps,
  cpp_args: python_binding_definitions,
  install: true,
  subdir: 'qtforge'
)

# Generate and install __init__.py
init_py_config = configuration_data()
init_py_config.set('VERSION_MAJOR', version_major)
init_py_config.set('VERSION_MINOR', version_minor)
init_py_config.set('VERSION_PATCH', version_patch)
init_py_config.set('VERSION_STRING', meson.project_version())

init_py = configure_file(
  input: '__init__.py.in',
  output: '__init__.py',
  configuration: init_py_config
)

python3.install_sources(init_py, subdir: 'qtforge')

# Install Python type stubs if available
python_stub_files = [
  '../../qtforge/__init__.pyi',
  '../../qtforge/core.pyi',
  '../../qtforge/utils.pyi'
]

foreach stub_file : python_stub_files
  if fs.exists(stub_file)
    python3.install_sources(stub_file, subdir: 'qtforge')
  endif
endforeach

# Create dependency for Python bindings
qtforge_python_dep = declare_dependency(
  dependencies: python_binding_deps
)
