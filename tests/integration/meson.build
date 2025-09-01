# Integration Tests Build Configuration

# Test sources - integration tests typically test full system functionality
integration_test_sources = [
  # Add integration test files here when they exist
]

# Only add test sources that actually exist
integration_existing_test_sources = []
foreach src : integration_test_sources
  if fs.exists(src)
    integration_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : integration_existing_test_sources
  test_name = test_src.split('.')[0]

  test_exe = executable(test_name,
    test_src,
    dependencies: [qtforge_core_dep, qt6_core_dep, qt6_network_dep, qt6_widgets_dep],
    include_directories: test_inc,
    cpp_args: test_cpp_args,
    install: false
  )

  # Register the test
  test(test_name, test_exe,
    suite: 'integration',
    timeout: 60
  )
endforeach

# If no integration tests exist yet, create a placeholder message
if integration_existing_test_sources.length() == 0
  message('No integration test sources found - integration tests will be skipped')
endif
