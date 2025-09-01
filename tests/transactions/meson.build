# Transactions Tests Build Configuration

# Test sources
transactions_test_sources = [
  'test_plugin_transaction_manager.cpp'
]

# Only add test sources that actually exist
transactions_existing_test_sources = []
foreach src : transactions_test_sources
  if fs.exists(src)
    transactions_existing_test_sources += [src]
  endif
endforeach

# Create test executables for each test file
foreach test_src : transactions_existing_test_sources
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
    suite: 'transactions',
    timeout: 30
  )
endforeach
