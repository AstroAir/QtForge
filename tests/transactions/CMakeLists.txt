# QtForge Transactions Module Tests
cmake_minimum_required(VERSION 3.21)

# Plugin Transaction Manager Tests
add_executable(test_plugin_transaction_manager
    test_plugin_transaction_manager.cpp
)

target_include_directories(test_plugin_transaction_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_transaction_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtForgeCore
)

add_test(NAME TransactionsPluginTests COMMAND test_plugin_transaction_manager)
set_tests_properties(TransactionsPluginTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test executables
install(TARGETS
    test_plugin_transaction_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests/transactions
)
