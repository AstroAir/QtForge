# Communication Tests Build Configuration

# Test sources
communication_test_sources = [
  'test_message_bus.cpp',
  'test_message_bus_simple.cpp',
  'test_service_contracts.cpp'
]

# Only add test sources that actually exist
communication_existing_test_sources = []
foreach src : communication_test_sources
  if fs.exists(src)
    communication_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : communication_existing_test_sources
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
    suite: 'communication',
    timeout: 30
  )
endforeach
