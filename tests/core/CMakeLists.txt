# QtForge Core Module Tests
cmake_minimum_required(VERSION 3.21)

# Plugin Interface Tests
add_executable(test_plugin_interface test_plugin_interface.cpp)

target_include_directories(test_plugin_interface
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(test_plugin_interface PRIVATE Qt6::Test Qt6::Core
                                                    QtForgeCore)

add_test(
  NAME CorePluginInterfaceTests
  COMMAND
    powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_interface.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(CorePluginInterfaceTests PROPERTIES TIMEOUT 60)

# Advanced Plugin Interface Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
  add_executable(test_plugin_interface_advanced
                 test_plugin_interface_advanced.cpp)

  target_include_directories(test_plugin_interface_advanced
                             PRIVATE ${CMAKE_SOURCE_DIR}/include)

  target_link_libraries(test_plugin_interface_advanced
                        PRIVATE Qt6::Test Qt6::Core QtForgeCore)

  add_test(
    NAME CorePluginInterfaceAdvancedTests
    COMMAND
      powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
      -TestExecutable
      ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_interface_advanced.exe
      -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(CorePluginInterfaceAdvancedTests PROPERTIES TIMEOUT 120)
endif()

# Plugin Manager Tests
add_executable(test_plugin_manager test_plugin_manager.cpp)

target_include_directories(test_plugin_manager
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(test_plugin_manager PRIVATE Qt6::Test Qt6::Core
                                                  QtForgeCore)

add_test(
  NAME CorePluginManagerTests
  COMMAND
    powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_manager.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(CorePluginManagerTests PROPERTIES TIMEOUT 120)

# Advanced Plugin Manager Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
  add_executable(test_plugin_manager_advanced test_plugin_manager_advanced.cpp)

  target_include_directories(test_plugin_manager_advanced
                             PRIVATE ${CMAKE_SOURCE_DIR}/include)

  target_link_libraries(test_plugin_manager_advanced
                        PRIVATE Qt6::Test Qt6::Core QtForgeCore)

  add_test(
    NAME CorePluginManagerAdvancedTests
    COMMAND
      powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
      -TestExecutable
      ${CMAKE_CURRENT_BINARY_DIR}/test_plugin_manager_advanced.exe
      -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(CorePluginManagerAdvancedTests PROPERTIES TIMEOUT 180)
endif()

# Lifecycle Manager Tests
add_executable(test_lifecycle_manager test_lifecycle_manager.cpp)

target_include_directories(test_lifecycle_manager
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(test_lifecycle_manager PRIVATE Qt6::Test Qt6::Core
                                                     QtForgeCore)

add_test(
  NAME CoreLifecycleManagerTests
  COMMAND
    powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_lifecycle_manager.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(CoreLifecycleManagerTests PROPERTIES TIMEOUT 60)

# Simple Lifecycle Tests
add_executable(test_lifecycle_simple test_lifecycle_simple.cpp)

target_include_directories(test_lifecycle_simple
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(test_lifecycle_simple PRIVATE Qt6::Test Qt6::Core
                                                    QtForgeCore)

add_test(
  NAME CoreLifecycleSimpleTests
  COMMAND
    powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_lifecycle_simple.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(CoreLifecycleSimpleTests PROPERTIES TIMEOUT 30)

# Lifecycle Compilation Tests
add_executable(test_lifecycle_compilation test_lifecycle_compilation.cpp)

target_include_directories(test_lifecycle_compilation
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(test_lifecycle_compilation PRIVATE Qt6::Test Qt6::Core
                                                         QtForgeCore)

add_test(
  NAME CoreLifecycleCompilationTests
  COMMAND
    powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_lifecycle_compilation.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(CoreLifecycleCompilationTests PROPERTIES TIMEOUT 30)

# Component Architecture Tests
add_executable(test_component_architecture test_component_architecture.cpp)

target_include_directories(test_component_architecture
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(test_component_architecture
                      PRIVATE Qt6::Test Qt6::Core Qt6::Concurrent QtForgeCore)

add_test(
  NAME CoreComponentArchitectureTests
  COMMAND
    powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_component_architecture.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(CoreComponentArchitectureTests PROPERTIES TIMEOUT 180)

# Advanced Plugin Interface Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
  add_executable(test_advanced_plugin_interface
                 test_advanced_plugin_interface.cpp)

  target_include_directories(
    test_advanced_plugin_interface PRIVATE ${CMAKE_SOURCE_DIR}/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/..)

  target_link_libraries(test_advanced_plugin_interface
                        PRIVATE Qt6::Test Qt6::Core QtForgeCore)

  add_test(
    NAME CoreAdvancedPluginInterfaceTests
    COMMAND
      powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
      -TestExecutable
      ${CMAKE_CURRENT_BINARY_DIR}/test_advanced_plugin_interface.exe
      -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties(CoreAdvancedPluginInterfaceTests
                       PROPERTIES TIMEOUT 120 LABELS "core;advanced")

  # Dynamic Plugin Interface Tests - DISABLED (API has changed - IDynamicPlugin
  # no longer exists) add_executable(test_dynamic_plugin_interface
  # test_dynamic_plugin_interface.cpp )
  #
  # target_include_directories(test_dynamic_plugin_interface PRIVATE
  # ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/.. )
  #
  # target_link_libraries(test_dynamic_plugin_interface PRIVATE Qt6::Test
  # Qt6::Core QtForgeCore )
  #
  # add_test(NAME CoreDynamicPluginInterfaceTests COMMAND powershell
  # -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
  # -TestExecutable
  # ${CMAKE_CURRENT_BINARY_DIR}/test_dynamic_plugin_interface.exe
  # -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR} )
  # set_tests_properties(CoreDynamicPluginInterfaceTests PROPERTIES TIMEOUT 120
  # LABELS "core;dynamic;advanced" )
endif()

# Install test executables
install(
  TARGETS test_plugin_interface test_plugin_manager test_lifecycle_manager
          test_lifecycle_simple test_lifecycle_compilation
          test_component_architecture
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/core)

if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
  install(
    TARGETS test_plugin_interface_advanced test_plugin_manager_advanced
            test_advanced_plugin_interface
            # test_dynamic_plugin_interface  # Disabled - API changed
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/core)
endif()
