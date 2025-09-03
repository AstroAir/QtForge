# QtForge Security Module Tests
cmake_minimum_required(VERSION 3.21)

# Security Manager Tests
add_executable(test_security_manager
    test_security_manager_simple.cpp
)

target_include_directories(test_security_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_security_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
    QtForgeSecurity
)

add_test(NAME SecurityManagerTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_security_manager.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(SecurityManagerTests PROPERTIES
    TIMEOUT 120
)

# Comprehensive Security Manager Tests - temporarily disabled due to linking errors
# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     add_executable(test_security_manager_comprehensive
#         test_security_manager.cpp
#     )
#
#     target_include_directories(test_security_manager_comprehensive PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#     )
#
#     target_link_libraries(test_security_manager_comprehensive PRIVATE
#         Qt6::Test
#         Qt6::Core
#         QtForgeCore
#         QtForgeSecurity
#     )
#
#     add_test(NAME SecurityManagerComprehensiveTests COMMAND test_security_manager_comprehensive)
#     set_tests_properties(SecurityManagerComprehensiveTests PROPERTIES
#         TIMEOUT 180
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR};D:/msys64/mingw64/bin;$ENV{PATH}"
#     )
# endif()

# Install test executables
install(TARGETS
    test_security_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/security
)

# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     install(TARGETS
#         test_security_manager_comprehensive
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/security
#     )
# endif()
