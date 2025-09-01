# Utils Tests Build Configuration

# Test sources
utils_test_sources = [
  'test_version.cpp',
  'test_version_simple.cpp',
  'test_error_handling.cpp',
  'test_error_handling_simple.cpp',
  'test_expected_comprehensive.cpp'
]

# Only add test sources that actually exist
utils_existing_test_sources = []
foreach src : utils_test_sources
  if fs.exists(src)
    utils_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : utils_existing_test_sources
  test_name = test_src.split('.')[0]

  # For Qt Test files that include MOC directly in cpp files
  test_moc_files = []
  if fs.exists(test_src)
    test_moc_files = qt6.compile_moc(
      sources: [test_src],
      dependencies: [qtforge_core_dep, qt6_core_dep],
      include_directories: test_inc
    )
  endif

  test_exe = executable(test_name,
    test_src,
    test_moc_files,
    dependencies: [qtforge_core_dep, qt6_core_dep, qt6_test_dep],
    include_directories: test_inc,
    cpp_args: test_cpp_args,
    install: false
  )

  # Register the test
  test(test_name, test_exe,
    suite: 'utils',
    timeout: 30
  )
endforeach
