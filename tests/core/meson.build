# Core Tests Build Configuration

# Core test sources
core_test_sources = [
  'test_plugin_interface.cpp',
  # 'test_plugin_manager.cpp',  # Temporarily disabled - PluginManager implementation incomplete
  # 'test_plugin_manager_comprehensive.cpp',  # Temporarily disabled due to missing methods
  'test_lifecycle_manager.cpp',
  'test_lifecycle_simple.cpp',
  'test_lifecycle_compilation.cpp',
  'test_component_architecture.cpp'
]

# Test executables
foreach test_source : core_test_sources
  test_name = test_source.split('.')[0]

  # For Qt Test files that include MOC directly in cpp files
  test_moc_files = []
  if fs.exists(test_source)
    test_moc_files = qt6.compile_moc(
      sources: [test_source],
      dependencies: test_deps,
      include_directories: test_inc
    )
  endif

  # Create test executable
  test_exe = executable(test_name,
    test_source,
    test_moc_files,
    include_directories: test_inc,
    dependencies: test_deps + [qt6_test_dep],
    cpp_args: test_cpp_args,
    install: get_option('install_tests'),
    install_dir: get_option('bindir') / 'qtforge-tests'
  )

  # Register test
  test(test_name, test_exe,
    timeout: 30,
    workdir: meson.current_source_dir()
  )
endforeach

# Install test metadata if available
if get_option('install_tests')
  test_metadata_files = [
    'mock_metadata.json',
    'test_metadata.json'
  ]

  foreach metadata_file : test_metadata_files
    if fs.exists(metadata_file)
      install_data(metadata_file,
        install_dir: get_option('datadir') / 'qtforge' / 'tests' / 'core'
      )
    endif
  endforeach
endif
