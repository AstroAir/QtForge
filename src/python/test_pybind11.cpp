// Simple test file to verify pybind11 compilation
#include <pybind11/pybind11.h>

PYBIND11_MODULE(test_module, m) {
    m.doc() = "Test module for pybind11 compilation";
    m.def("test_function", []() { return "Hello from pybind11!"; });
}
