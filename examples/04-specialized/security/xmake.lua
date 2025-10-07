-- Security example (console)
add_rules("qt.console")
add_packages("qt6core")

target("security_example")
    set_kind("binary")
    add_files("security_example.cpp")
    add_deps("QtForgeCore")
    set_languages("c++20")

