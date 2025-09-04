# QtForge Python Module Tests

# Python tests require Python development headers, pybind11, and Python bindings to be enabled
if get_option('python_bindings') and python3_dep.found() and pybind11_dep.found()
  
  # Python Binding Tests
  test_python_bindings = executable('test_python_bindings',
    'test_python_bindings.cpp',
    include_directories: [inc_dir],
    dependencies: [
      qt6_test_dep,
      qt6_core_dep,
      qtforge_core_dep
    ],
    install: get_option('install_tests'),
    install_dir: get_option('bindir') / 'tests' / 'python'
  )

  # Add test
  test('PythonBindingTests', test_python_bindings,
    timeout: 120,
    workdir: meson.current_build_dir()
  )

  # Minimal Python test
  if fs.exists('test_minimal.cpp')
    test_minimal = executable('test_minimal',
      'test_minimal.cpp',
      include_directories: [inc_dir],
      dependencies: [
        qt6_test_dep,
        qt6_core_dep,
        qtforge_core_dep
      ],
      install: get_option('install_tests'),
      install_dir: get_option('bindir') / 'tests' / 'python'
    )

    test('PythonMinimalTest', test_minimal,
      timeout: 60,
      workdir: meson.current_build_dir()
    )
  endif

else
  message('Python binding tests disabled - requires python_bindings=true, Python3, and pybind11')
endif
