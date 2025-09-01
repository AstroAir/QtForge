# QtForge Examples Build Configuration

# Only build examples if explicitly requested
if not get_option('build_examples')
  subdir_done()
endif

message('Building QtForge examples')

# Check for required dependencies for examples
examples_qt_deps = [qt6_core_dep]
if qt6_widgets_dep.found()
  examples_qt_deps += [qt6_widgets_dep]
endif
if qt6_network_dep.found()
  examples_qt_deps += [qt6_network_dep]
endif

# Example build helper function
examples_inc = [inc_dir, include_directories('.')]

# Basic examples (only include if meson.build exists)
if fs.exists('01-fundamentals/basic-plugin/meson.build')
  subdir('01-fundamentals/basic-plugin')
endif

if fs.exists('01-fundamentals/hello-world/meson.build')
  subdir('01-fundamentals/hello-world')
endif

# Communication examples (conditional)
if fs.exists('02-communication/message-passing/meson.build')
  subdir('02-communication/message-passing')
endif

if fs.exists('02-communication/request-response/meson.build')
  subdir('02-communication/request-response')
endif

# Service examples (conditional)
if fs.exists('03-services/background-tasks/meson.build')
  subdir('03-services/background-tasks')
endif

if fs.exists('03-services/service-discovery/meson.build')
  subdir('03-services/service-discovery')
endif

# Specialized examples (conditional on Qt components and file existence)
if qt6_widgets_dep.found() and fs.exists('04-specialized/ui-integration/meson.build')
  subdir('04-specialized/ui-integration')
endif

if qt6_widgets_dep.found() and fs.exists('04-specialized/theme-system/meson.build')
  subdir('04-specialized/theme-system')
endif

if qt6_network_dep.found() and fs.exists('04-specialized/network-plugins/meson.build')
  subdir('04-specialized/network-plugins')
endif

if qt6_sql_dep.found() and fs.exists('04-specialized/database-plugins/meson.build')
  subdir('04-specialized/database-plugins')
endif

# Integration examples (conditional)
if fs.exists('05-integration/multi-plugin-app/meson.build')
  subdir('05-integration/multi-plugin-app')
endif

if fs.exists('05-integration/plugin-dependencies/meson.build')
  subdir('05-integration/plugin-dependencies')
endif

# Comprehensive examples (require multiple Qt components and file existence)
if qt6_widgets_dep.found() and qt6_network_dep.found() and fs.exists('06-comprehensive/full-application/meson.build')
  subdir('06-comprehensive/full-application')
endif

# Install examples if requested
if get_option('install_examples')
  # Install example source files
  install_subdir('01-fundamentals',
    install_dir: get_option('datadir') / 'qtforge' / 'examples',
    strip_directory: false
  )

  install_subdir('02-communication',
    install_dir: get_option('datadir') / 'qtforge' / 'examples',
    strip_directory: false
  )

  install_subdir('03-services',
    install_dir: get_option('datadir') / 'qtforge' / 'examples',
    strip_directory: false
  )

  install_subdir('04-specialized',
    install_dir: get_option('datadir') / 'qtforge' / 'examples',
    strip_directory: false
  )

  install_subdir('05-integration',
    install_dir: get_option('datadir') / 'qtforge' / 'examples',
    strip_directory: false
  )

  install_subdir('06-comprehensive',
    install_dir: get_option('datadir') / 'qtforge' / 'examples',
    strip_directory: false
  )

  # Install example documentation
  install_data('README.md',
    install_dir: get_option('datadir') / 'qtforge' / 'examples'
  )
endif
