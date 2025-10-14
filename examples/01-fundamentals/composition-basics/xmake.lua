-- Composition basics example (console)
add_rules("qt.console")
add_packages("qt6core")

target("composition_basics")
    set_kind("binary")
    add_files("composition_basics.cpp")
    add_deps("QtForgeCore")
    set_languages("c++20")
