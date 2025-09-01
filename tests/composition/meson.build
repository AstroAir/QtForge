# Composition Tests Build Configuration

# Test sources
composition_test_sources = [
  'test_plugin_composition.cpp'
]

# Only add test sources that actually exist
composition_existing_test_sources = []
foreach src : composition_test_sources
  if fs.exists(src)
    composition_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : composition_existing_test_sources
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
    suite: 'composition',
    timeout: 30
  )
endforeach
