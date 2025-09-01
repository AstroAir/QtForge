# Security Tests Build Configuration

# Test sources
security_test_sources = [
  'test_security_manager.cpp',
  'test_security_manager_simple.cpp',
  'test_plugin_sandbox.cpp',
  'test_resource_monitor.cpp',
  'test_sandbox_integration.cpp',
  'test_sandbox_manager.cpp',
  'test_sandbox_performance.cpp',
  'test_security_enforcer.cpp'
]

# Only add test sources that actually exist
security_existing_test_sources = []
foreach src : security_test_sources
  if fs.exists(src)
    security_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : security_existing_test_sources
  test_name = test_src.split('.')[0]

  # For Qt Test files that include MOC directly in cpp files
  test_moc_files = []
  if fs.exists(test_src)
    test_moc_files = qt6.compile_moc(
      sources: [test_src],
      dependencies: [qtforge_core_dep, qtforge_security_dep, qt6_core_dep],
      include_directories: test_inc
    )
  endif

  test_exe = executable(test_name,
    test_src,
    test_moc_files,
    dependencies: [qtforge_core_dep, qtforge_security_dep, qt6_core_dep, qt6_test_dep],
    include_directories: test_inc,
    cpp_args: test_cpp_args,
    install: false
  )

  # Register the test
  test(test_name, test_exe,
    suite: 'security',
    timeout: 60
  )
endforeach
