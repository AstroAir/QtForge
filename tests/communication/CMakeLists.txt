# QtForge Communication Module Tests
cmake_minimum_required(VERSION 3.21)

# Message Bus Simple Tests
add_executable(test_message_bus_simple
    test_message_bus_simple.cpp
)

target_include_directories(test_message_bus_simple PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(test_message_bus_simple PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME CommunicationMessageBusSimpleTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_message_bus_simple.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(CommunicationMessageBusSimpleTests PROPERTIES
    TIMEOUT 120
    LABELS "communication;message_bus;simple"
)

# Comprehensive Message Bus Tests
if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    add_executable(test_message_bus_comprehensive
        test_message_bus.cpp
    )

    target_include_directories(test_message_bus_comprehensive PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_link_libraries(test_message_bus_comprehensive PRIVATE
        Qt6::Test
        Qt6::Core
        QtForgeCore
    )

    add_test(NAME CommunicationMessageBusComprehensiveTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_message_bus_comprehensive.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(CommunicationMessageBusComprehensiveTests PROPERTIES
        TIMEOUT 180
        LABELS "communication;message_bus;comprehensive"
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

    add_test(NAME CommunicationServiceContractsTests
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
        -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_service_contracts.exe
        -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(CommunicationServiceContractsTests PROPERTIES
        TIMEOUT 60
        LABELS "communication;service_contracts;comprehensive"
    )
endif()

# Request-Response System Tests
add_executable(test_request_response_system
    test_request_response_system.cpp
)

target_include_directories(test_request_response_system PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(test_request_response_system PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME CommunicationRequestResponseTests
    COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/run_test.ps1
    -TestExecutable ${CMAKE_CURRENT_BINARY_DIR}/test_request_response_system.exe
    -WorkingDirectory ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(CommunicationRequestResponseTests PROPERTIES
    TIMEOUT 120
    LABELS "communication;request_response"
)

# Install test executables
set(COMMUNICATION_TEST_TARGETS test_message_bus_simple test_request_response_system)

if(QTFORGE_BUILD_COMPREHENSIVE_TESTS)
    list(APPEND COMMUNICATION_TEST_TARGETS test_message_bus_comprehensive test_service_contracts)
endif()

install(TARGETS ${COMMUNICATION_TEST_TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/communication
)
