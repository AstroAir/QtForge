# Managers Tests Build Configuration

# Test sources
managers_test_sources = [
  'test_configuration_manager.cpp',
  'test_manager_component_integration.cpp',
  'test_plugin_version_manager.cpp',
  'test_resource_management.cpp'
]

# Only add test sources that actually exist
managers_existing_test_sources = []
foreach src : managers_test_sources
  if fs.exists(src)
    managers_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : managers_existing_test_sources
  test_name = test_src.split('.')[0]

  test_exe = executable(test_name,
    test_src,
    dependencies: [qtforge_core_dep, qt6_core_dep, qt6_test_dep],
    include_directories: test_inc,
    cpp_args: test_cpp_args,
    install: false
  )

  # Register the test
  test(test_name, test_exe,
    suite: 'managers',
    timeout: 30
  )
endforeach
