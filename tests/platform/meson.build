# Platform Tests Build Configuration

# Test sources
platform_test_sources = [
  'test_cross_platform.cpp'
]

# Only add test sources that actually exist
platform_existing_test_sources = []
foreach src : platform_test_sources
  if fs.exists(src)
    platform_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : platform_existing_test_sources
  test_name = test_src.split('.')[0]
  
  test_exe = executable(test_name,
    test_src,
    dependencies: [qtforge_core_dep, qt6_core_dep],
    include_directories: test_inc,
    cpp_args: test_cpp_args,
    install: false
  )
  
  # Register the test
  test(test_name, test_exe,
    suite: 'platform',
    timeout: 30
  )
endforeach
