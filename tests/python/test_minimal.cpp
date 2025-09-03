#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(test_minimal, m) {
    m.doc() = "pybind11 minimal test plugin";
    m.def("add", &add, "A function which adds two numbers");
}
