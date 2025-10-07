cmake_minimum_required(VERSION 3.21)
project(QtForgeTransactionsExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED COMPONENTS Core)

add_executable(transactions_example
    transactions_example.cpp
)

target_link_libraries(transactions_example PRIVATE
    Qt6::Core
    QtForge::Core
)

