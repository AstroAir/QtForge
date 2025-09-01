# Basic Plugin Example Build Configuration

# Plugin sources
basic_plugin_sources = [
  'basic_plugin.cpp'
]

# Plugin headers (may need MOC processing)
basic_plugin_headers = [
  'basic_plugin.hpp'
]

# Check if headers need MOC processing
basic_plugin_moc_files = []
if fs.exists('basic_plugin.hpp')
  # Only process MOC if the header contains Q_OBJECT
  basic_plugin_moc_files = qt6.compile_moc(
    headers: basic_plugin_headers,
    dependencies: [qt6_core_dep, qtforge_core_dep],
    include_directories: [examples_inc]
  )
endif

# Plugin metadata
basic_plugin_metadata = [
  'basic_plugin.json'
]

# Create the plugin as a shared library
basic_plugin = shared_library('basic_plugin',
  basic_plugin_sources,
  basic_plugin_moc_files,
  include_directories: examples_inc,
  dependencies: [qtforge_core_dep, qt6_core_dep],
  name_prefix: '',  # Remove 'lib' prefix
  name_suffix: 'qtplugin',  # Use .qtplugin extension
  install: get_option('install_examples'),
  install_dir: get_option('libdir') / 'qtforge' / 'plugins'
)

# Install plugin metadata
if get_option('install_examples')
  install_data(basic_plugin_metadata,
    install_dir: get_option('libdir') / 'qtforge' / 'plugins'
  )
endif

# Create alias for the plugin
basic_plugin_dep = declare_dependency(
  link_with: basic_plugin,
  dependencies: [qtforge_core_dep, qt6_core_dep]
)
