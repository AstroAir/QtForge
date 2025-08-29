# QtForge Communication Module Tests
cmake_minimum_required(VERSION 3.21)

# Message Bus Tests
add_executable(test_message_bus
    test_message_bus_simple.cpp
)

target_include_directories(test_message_bus PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_message_bus PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME CommunicationMessageBusTests COMMAND test_message_bus)
set_tests_properties(CommunicationMessageBusTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Comprehensive Message Bus Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_message_bus_comprehensive
        test_message_bus.cpp
    )

    target_include_directories(test_message_bus_comprehensive PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_message_bus_comprehensive PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME CommunicationMessageBusComprehensiveTests COMMAND test_message_bus_comprehensive)
    set_tests_properties(CommunicationMessageBusComprehensiveTests PROPERTIES
        TIMEOUT 180
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Service Contracts Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_service_contracts
        test_service_contracts.cpp
    )

    target_include_directories(test_service_contracts PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(test_service_contracts PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME CommunicationServiceContractsTests COMMAND test_service_contracts)
    set_tests_properties(CommunicationServiceContractsTests PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Install test executables
install(TARGETS 
    test_message_bus
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/communication
)

if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    install(TARGETS 
        test_message_bus_comprehensive
        test_service_contracts
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/communication
    )
endif()
