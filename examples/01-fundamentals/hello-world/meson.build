# Hello World Plugin Example Build Configuration

# Plugin sources
hello_world_sources = [
  'hello_world_plugin.cpp'
]

# Plugin headers (may need MOC processing)
hello_world_headers = [
  'hello_world_plugin.hpp'
]

# Check if headers need MOC processing
hello_world_moc_files = []
if fs.exists('hello_world_plugin.hpp')
  # Only process MOC if the header contains Q_OBJECT
  hello_world_moc_files = qt6.compile_moc(
    headers: hello_world_headers,
    dependencies: [qt6_core_dep, qtforge_core_dep],
    include_directories: [examples_inc]
  )
endif

# Plugin metadata
hello_world_metadata = [
  'hello_world_plugin.json'
]

# Create the plugin as a shared library
hello_world_plugin = shared_library('hello_world_plugin',
  hello_world_sources,
  hello_world_moc_files,
  include_directories: examples_inc,
  dependencies: [qtforge_core_dep, qt6_core_dep],
  name_prefix: '',  # Remove 'lib' prefix
  name_suffix: 'qtplugin',  # Use .qtplugin extension
  install: get_option('install_examples'),
  install_dir: get_option('libdir') / 'qtforge' / 'plugins'
)

# Install plugin metadata
if get_option('install_examples')
  install_data(hello_world_metadata,
    install_dir: get_option('libdir') / 'qtforge' / 'plugins'
  )
endif

# Create alias for the plugin
hello_world_plugin_dep = declare_dependency(
  link_with: hello_world_plugin,
  dependencies: [qtforge_core_dep, qt6_core_dep]
)
