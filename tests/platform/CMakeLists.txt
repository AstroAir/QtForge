# QtForge Platform Module Tests
cmake_minimum_required(VERSION 3.21)

# Cross-Platform Tests
if(QTFORGE_BUILD_CROSS_PLATFORM_TESTS)
    add_executable(test_cross_platform
        test_cross_platform.cpp
    )

    target_include_directories(test_cross_platform PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_cross_platform PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME PlatformCrossPlatformTests COMMAND test_cross_platform)
    set_tests_properties(PlatformCrossPlatformTests PROPERTIES
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Install test executables
    install(TARGETS 
        test_cross_platform
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/platform
    )
endif()
