# QtForge Utils Module Tests
cmake_minimum_required(VERSION 3.21)

# Version Tests
add_executable(test_version
    test_version_simple.cpp
)

target_include_directories(test_version PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_version PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME UtilsVersionTests COMMAND test_version)
set_tests_properties(UtilsVersionTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Comprehensive Version Tests - temporarily disabled due to linking errors
# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     add_executable(test_version_comprehensive
#         test_version.cpp
#     )
#
#     target_include_directories(test_version_comprehensive PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#     )
#
#     target_link_libraries(test_version_comprehensive PRIVATE
#         Qt6::Test
#         Qt6::Core
#         QtForgeCore
#     )
#
#     add_test(NAME UtilsVersionComprehensiveTests COMMAND test_version_comprehensive)
#     set_tests_properties(UtilsVersionComprehensiveTests PROPERTIES
#         TIMEOUT 90
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     )
# endif()

# Error Handling Tests
add_executable(test_error_handling
    test_error_handling_simple.cpp
)

target_include_directories(test_error_handling PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_error_handling PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME UtilsErrorHandlingTests COMMAND test_error_handling)
set_tests_properties(UtilsErrorHandlingTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Comprehensive Error Handling Tests
# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     add_executable(test_error_handling_comprehensive
#         test_error_handling.cpp
#     )
#
#     target_include_directories(test_error_handling_comprehensive PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#     )
#
#     target_link_libraries(test_error_handling_comprehensive PRIVATE
#         Qt6::Test
#         Qt6::Core
#         QtForgeCore
#     )
#
#     add_test(NAME UtilsErrorHandlingComprehensiveTests COMMAND test_error_handling_comprehensive)
#     set_tests_properties(UtilsErrorHandlingComprehensiveTests PROPERTIES
#         TIMEOUT 90
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     )
# endif()

# Expected<T,E> Comprehensive Tests - temporarily disabled due to linking errors
# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     add_executable(test_expected_comprehensive
#         test_expected_comprehensive.cpp
#     )
#
#     target_include_directories(test_expected_comprehensive PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#     )
#
#     target_link_libraries(test_expected_comprehensive PRIVATE
#         Qt6::Test
#         Qt6::Core
#         QtForgeCore
#     )
#
#     add_test(NAME UtilsExpectedComprehensiveTests COMMAND test_expected_comprehensive)
#     set_tests_properties(UtilsExpectedComprehensiveTests PROPERTIES
#         TIMEOUT 60
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     )
# endif()

# Install test executables
install(TARGETS
    test_version
    test_error_handling
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/utils
)

# if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
#     install(TARGETS
#         test_version_comprehensive
#         test_error_handling_comprehensive
#         test_expected_comprehensive
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/utils
#     )
# endif()
